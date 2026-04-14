using Dapper;
using fb.protocol.marketplace;
using Http.Extension;
using Http.Service;
using StackExchange.Redis;

namespace Marketplace.Services
{
    public class MarketplaceArchiveBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<MarketplaceArchiveBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(10);
        private static readonly TimeSpan RetryInterval = TimeSpan.FromMinutes(1);
        private const string LockKey = "fb:marketplace:archive:lock";

        public MarketplaceArchiveBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            ILogger<MarketplaceArchiveBackgroundService> logger)
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
                            var logService = scope.ServiceProvider.GetRequiredService<LogService>();

                            // Archive listings
                            await ArchiveListingsAsync(dbContext, logService, stoppingToken);
                        }
                        catch (Exception e)
                        {
                            _logger.LogError(e, $"Error processing marketplace archive: {e.Message}");
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
                    _logger.LogError(e, $"Error in marketplace archive background service: {e.Message}");

                    // Wait on error as well
                    await Task.Delay(RetryInterval, stoppingToken);
                }
            }
        }

        private async Task ArchiveListingsAsync(
            Marketplace.Service.DbContext dbContext,
            LogService logService,
            CancellationToken cancellationToken)
        {
            await using var conn = dbContext.GetUnifiedConnection();
            await conn.OpenAsync(cancellationToken);
            await using var transaction = await conn.BeginTransactionAsync(cancellationToken);

            try
            {
                // Archive all non-active listings in a single query
                var archiveSql = $@"
                    INSERT INTO `marketplace_listing_archive` (
                        `id`,
                        `world`,
                        `seller_id`,
                        `item_model`,
                        `remaining_count`,
                        `item_durability`,
                        `item_custom_name`,
                        `price`,
                        `status`,
                        `expire_date`,
                        `created_date`,
                        `sold_date`,
                        `updated_date`,
                        `archived_date`)
                    SELECT 
                        `id`,
                        `world`,
                        `seller_id`,
                        `item_model`,
                        `remaining_count`,
                        `item_durability`,
                        `item_custom_name`,
                        `price`,
                        `status`,
                        `expire_date`,
                        `created_date`,
                        `sold_date`,
                        `updated_date`,
                        NOW() AS `archived_date`
                    FROM `marketplace_listing`
                    WHERE `status` != {ListingState.ACTIVE.Escape()}";

                var archivedCount = await conn.ExecuteAsync(archiveSql, null, transaction);

                if (archivedCount > 0)
                {
                    // Delete archived listings from original table
                    var deleteSql = $@"
                        DELETE FROM `marketplace_listing`
                        WHERE `status` != {ListingState.ACTIVE.Escape()}";

                    await conn.ExecuteAsync(deleteSql, null, transaction);
                }

                await transaction.CommitAsync(cancellationToken);

                // Log success
                logService?.Write("marketplace_archive_success", new
                {
                    archived_count = archivedCount
                });
            }
            catch (Exception ex)
            {
                await transaction.RollbackAsync(cancellationToken);
                _logger.LogError(ex, "Error archiving marketplace listings");
                logService?.Write("marketplace_archive_failed", new
                {
                    error = ex.Message
                });
                throw;
            }
        }
    }
}