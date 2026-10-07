using Dapper;
using fb.protocol.marketplace;
using Fb.Model;
using Http.Extension;
using Http.Service;
using Marketplace.Extension;
using Marketplace.Model;
using StackExchange.Redis;

namespace Marketplace.Services
{
    public class MarketplaceExpireBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<MarketplaceExpireBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(1);
        private static readonly TimeSpan RetryInterval = TimeSpan.FromSeconds(10);
        private const string LockKey = "fb:marketplace:expire:lock";
        private const int ExpireBatchSize = 1000;

        private static readonly string AcquireLockScript = """
            if redis.call('exists', KEYS[1]) == 0 then
                redis.call('set', KEYS[1], '1')
                redis.call('expire', KEYS[1], ARGV[1])
                return 1
            else
                return 0
            end
            """;

        public MarketplaceExpireBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            ILogger<MarketplaceExpireBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _redisService = redisService;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    // Use Lua script to atomically check and acquire lock
                    var redis = _redisService.GetUnifiedConnection();
                    var result = await redis.EvalAsync(
                        AcquireLockScript,
                        [LockKey],
                        [(int)ProcessingInterval.TotalSeconds]);

                    var lockAcquired = (int)result == 1;

                    if (lockAcquired)
                    {
                        try
                        {
                            // Create scope for service resolution
                            using var scope = _scopeFactory.CreateScope();
                            var dbContext = scope.ServiceProvider.GetRequiredService<Marketplace.Service.DbContext>();
                            var deliveryService = scope.ServiceProvider.GetRequiredService<MarketplaceDeliveryService>();
                            var logService = scope.ServiceProvider.GetRequiredService<LogService>();

                            // Process expired listings
                            await ProcessExpiredListingsAsync(dbContext, deliveryService, logService, stoppingToken);
                        }
                        catch (Exception e)
                        {
                            _logger.LogError(e, $"Error processing expired marketplace listings: {e.Message}");
                        }
                        // Lock will expire automatically via TTL

                        // Wait longer after successful processing
                        await Task.Delay(ProcessingInterval, stoppingToken);
                    }
                    else
                    {
                        // Wait shorter when lock acquisition fails
                        await Task.Delay(RetryInterval, stoppingToken);
                    }
                }
                catch (Exception e)
                {
                    _logger.LogError(e, $"Error in marketplace expire background service: {e.Message}");

                    // Wait on error as well
                    await Task.Delay(RetryInterval, stoppingToken);
                }
            }
        }

        private async Task ProcessExpiredListingsAsync(
            Marketplace.Service.DbContext dbContext,
            MarketplaceDeliveryService deliveryService,
            LogService logService,
            CancellationToken cancellationToken)
        {
            await using var conn = dbContext.GetUnifiedConnection();
            await conn.OpenAsync(cancellationToken);
            await using var transaction = await conn.BeginTransactionAsync(cancellationToken);
            var committed = false;

            try
            {
                // Lock expired active listings; rows held by cancel/purchase are left for the next run
                var sql = $@"
                    SELECT * FROM `marketplace_listing`
                    WHERE `status` = {ListingState.ACTIVE.Escape()}
                      AND `expire_date` < NOW()
                    ORDER BY `expire_date`
                    LIMIT {ExpireBatchSize}
                    FOR UPDATE SKIP LOCKED";

                var expiredListings = (await conn.QueryAsync<MarketplaceListing>(sql, null, transaction)).ToList();

                if (expiredListings.Count == 0)
                {
                    await transaction.CommitAsync(cancellationToken);
                    return;
                }

                // Update only the locked batch so every EXPIRED row gets its items returned below
                var parameters = new DynamicParameters();
                parameters.Add("ListingIds", expiredListings.Select(x => x.Id).ToList());

                var updateSql = $@"
                    UPDATE `marketplace_listing`
                    SET `status` = {ListingState.EXPIRED.Escape()},
                        `updated_date` = NOW()
                    WHERE `id` IN @ListingIds
                      AND `status` = {ListingState.ACTIVE.Escape()}";

                await conn.ExecuteAsync(updateSql, parameters, transaction);

                // Return items with registration fee; the delivery rows commit together with the EXPIRED status
                var deliveries = new List<MarketplaceDelivery>(expiredListings.Count);
                foreach (var listing in expiredListings)
                {
                    deliveries.Add(await CreateExpiredDeliveryAsync(listing, dbContext, transaction));
                }

                await transaction.CommitAsync(cancellationToken);
                committed = true;

                for (var i = 0; i < expiredListings.Count; i++)
                {
                    var listing = expiredListings[i];
                    var delivered = await deliveryService.DeliverAsync(deliveries[i]);
                    var totalPrice = listing.Price * listing.RemainingCount;
                    await logService.WriteAsync("marketplace_expired_listing_processed", new
                    {
                        listing_id = listing.Id,
                        seller_id = listing.SellerId,
                        item_model = listing.ItemModel,
                        remaining_count = listing.RemainingCount,
                        total_price = totalPrice,
                        registration_fee = (ulong)(totalPrice * Fb.Model.ConstValue.Marketplace.ListingFee),
                        delivered
                    });
                }

                // Log success
                await logService.WriteAsync("marketplace_expire_success", new
                {
                    expired_count = expiredListings.Count
                });
            }
            catch (Exception ex)
            {
                if (committed == false)
                {
                    await transaction.RollbackAsync(cancellationToken);
                }
                await logService.WriteAsync("marketplace_expire_failed", new
                {
                    error = ex.Message
                });
                throw;
            }
        }

        private async Task<MarketplaceDelivery> CreateExpiredDeliveryAsync(
            MarketplaceListing listing,
            Marketplace.Service.DbContext dbContext,
            System.Data.IDbTransaction transaction)
        {
            // Calculate registration fee that was paid when listing was created
            var totalPrice = listing.Price * listing.RemainingCount;
            var registrationFee = (ulong)(totalPrice * Fb.Model.ConstValue.Marketplace.ListingFee);

            // Get item name for message
            var itemName = "unknown item name";
            if (Table.Item.TryGetValue(listing.ItemModel, out var model))
            {
                itemName = model.Name;
            }

            // Create attachments: item + registration fee refund
            var attachments = new List<Fb.Model.Dsl>
            {
                new Fb.Model.Dsl.Item
                {
                    Id = listing.ItemModel,
                    Count = listing.RemainingCount,
                    Durability = listing.ItemDurability,
                    CustomName = listing.ItemCustomName,
                    Percent = 100.0,
                    Uid = listing.ItemUid
                }.ToDSL()
            };

            // Add registration fee refund
            if (registrationFee > 0)
            {
                attachments.Add(new Fb.Model.Dsl.Money { Value = registrationFee }.ToDSL());
            }

            var message = registrationFee > 0
                ? string.Format(Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredMessageWithFee.ToCSharpFormat(), itemName, listing.RemainingCount, registrationFee)
                : string.Format(Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredMessage.ToCSharpFormat(), itemName, listing.RemainingCount);

            var delivery = new MarketplaceDelivery
            {
                ExternalRef = $"marketplace:expire:{listing.Id}",
                World = listing.World,
                User = listing.SellerId,
                Title = Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredTitle,
                Message = message,
                Attachments = attachments
            };
            await dbContext.MarketplaceDelivery.CreateAsync(delivery, transaction);
            return delivery;
        }
    }
}