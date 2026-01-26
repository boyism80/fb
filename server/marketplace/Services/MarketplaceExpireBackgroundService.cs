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
    /// <summary>
    /// Provides a background service that periodically processes expired marketplace listings.
    /// Uses Redis TTL with Lua script for atomic distributed locking to prevent concurrent execution across multiple instances.
    /// </summary>
    public class MarketplaceExpireBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<MarketplaceExpireBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(1);
        private static readonly TimeSpan RetryInterval = TimeSpan.FromSeconds(10);
        private const string LockKey = "marketplace:expire:lock";
        private const int ExpireBatchSize = 1000;

        /// <summary>
        /// Initializes a new instance of the <see cref="MarketplaceExpireBackgroundService"/> class.
        /// </summary>
        public MarketplaceExpireBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            ILogger<MarketplaceExpireBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _redisService = redisService;
            _logger = logger;
        }

        /// <summary>
        /// Executes the background service that periodically processes expired marketplace listings.
        /// </summary>
        /// <param name="stoppingToken">The cancellation token for stopping the service.</param>
        /// <returns>A task representing the asynchronous execution of the background service.</returns>
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    // Use Lua script to atomically check and acquire lock
                    var redis = _redisService.Redis(LockKey);
                    var acquireLockScript = @"
                        if redis.call('exists', KEYS[1]) == 0 then
                            redis.call('set', KEYS[1], '1')
                            redis.call('expire', KEYS[1], ARGV[1])
                            return 1
                        else
                            return 0
                        end";
                    var script = LuaScript.Prepare(acquireLockScript);
                    var loadedScript = script.Load(redis.GetServer());
                    var result = await redis.Connection.ScriptEvaluateAsync(
                        loadedScript.Hash,
                        new RedisKey[] { LockKey },
                        new RedisValue[] { (int)ProcessingInterval.TotalSeconds });

                    var lockAcquired = (int)result == 1;

                    if (lockAcquired)
                    {
                        try
                        {
                            // Create scope for service resolution
                            using var scope = _scopeFactory.CreateScope();
                            var dbContext = scope.ServiceProvider.GetRequiredService<Marketplace.Service.DbContext>();
                            var storageService = scope.ServiceProvider.GetRequiredService<StorageService>();
                            var logService = scope.ServiceProvider.GetRequiredService<LogService>();

                            // Process expired listings
                            await ProcessExpiredListingsAsync(dbContext, storageService, logService, stoppingToken);
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

        /// <summary>
        /// Processes expired active listings by updating their status to EXPIRED and returning items with registration fee to sellers.
        /// </summary>
        /// <param name="dbContext">Database context for marketplace operations.</param>
        /// <param name="storageService">Storage service for creating pending boxes.</param>
        /// <param name="logService">Log service for logging operations.</param>
        /// <param name="cancellationToken">Cancellation token for the operation.</param>
        /// <returns>Task representing the asynchronous operation.</returns>
        private async Task ProcessExpiredListingsAsync(
            Marketplace.Service.DbContext dbContext,
            StorageService storageService,
            LogService logService,
            CancellationToken cancellationToken)
        {
            await using var conn = dbContext.Connection(-1);
            await conn.OpenAsync(cancellationToken);
            await using var transaction = await conn.BeginTransactionAsync(cancellationToken);

            try
            {
                // Get expired active listings
                var sql = $@"
                    SELECT * FROM `marketplace_listing`
                    WHERE `status` = {ListingState.ACTIVE.Escape()}
                      AND `expire_date` < NOW()
                    LIMIT {ExpireBatchSize}";

                var expiredListings = (await conn.QueryAsync<MarketplaceListing>(sql, null, transaction)).ToList();

                if (expiredListings.Count == 0)
                {
                    await transaction.CommitAsync(cancellationToken);
                    return;
                }

                // Update status to EXPIRED
                var updateSql = $@"
                    UPDATE `marketplace_listing`
                    SET `status` = {ListingState.EXPIRED.Escape()},
                        `updated_date` = NOW()
                    WHERE `status` = {ListingState.ACTIVE.Escape()}
                      AND `expire_date` < NOW()";

                await conn.ExecuteAsync(updateSql, null, transaction);

                await transaction.CommitAsync(cancellationToken);

                // Process each expired listing to return items with registration fee
                // This is done after transaction commit
                foreach (var listing in expiredListings)
                {
                    await ProcessExpiredListingAsync(listing, storageService, logService, cancellationToken);
                }

                // Log success
                logService?.Write("marketplace_expire_success", new
                {
                    expired_count = expiredListings.Count
                });
            }
            catch (Exception ex)
            {
                await transaction.RollbackAsync(cancellationToken);
                logService?.Write("marketplace_expire_failed", new
                {
                    error = ex.Message
                });
                throw;
            }
        }

        /// <summary>
        /// Processes an expired listing by returning items and registration fee to the seller.
        /// </summary>
        /// <param name="listing">The expired listing to process.</param>
        /// <param name="storageService">Storage service for creating pending boxes.</param>
        /// <param name="logService">Log service for logging operations.</param>
        /// <param name="cancellationToken">Cancellation token for the operation.</param>
        /// <returns>Task representing the asynchronous operation.</returns>
        private async Task ProcessExpiredListingAsync(
            MarketplaceListing listing,
            StorageService storageService,
            LogService logService,
            CancellationToken cancellationToken)
        {
            // Calculate registration fee that was paid when listing was created
            var totalPrice = listing.Price * listing.RemainingCount;
            var registrationFee = (uint)(totalPrice * Fb.Model.ConstValue.Marketplace.ListingFee);

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
                    Percent = 100.0
                }.ToDSL()
            };

            // Add registration fee refund
            if (registrationFee > 0)
            {
                attachments.Add(new Fb.Model.Dsl.Money { Value = registrationFee }.ToDSL());
            }

            // Use StorageService to create pending box (handles Redis caching and DB write-back automatically)
            var message = registrationFee > 0
                ? string.Format(Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredMessageWithFee.ToCSharpFormat(), itemName, listing.RemainingCount, registrationFee)
                : string.Format(Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredMessage.ToCSharpFormat(), itemName, listing.RemainingCount);

            await storageService.CreatePendingAsync(
                listing.Section,
                Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredTitle,
                message,
                listing.SellerId,
                null, // Unlimited expiry for marketplace items
                attachments);

            // Log expired listing processing
            logService?.Write("marketplace_expired_listing_processed", new
            {
                listing_id = listing.Id,
                seller_id = listing.SellerId,
                item_model = listing.ItemModel,
                remaining_count = listing.RemainingCount,
                total_price = totalPrice,
                registration_fee = registrationFee
            });
        }
    }
}
