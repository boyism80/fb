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
        private static readonly TimeSpan Retention = Fb.Model.ConstValue.Marketplace.ExpireTime;
        private const int BatchSize = 500;
        private const string LockKey = "fb:marketplace:archive:lock";

        private static readonly string AcquireLockScript = """
            if redis.call('exists', KEYS[1]) == 0 then
                redis.call('set', KEYS[1], '1')
                redis.call('expire', KEYS[1], ARGV[1])
                return 1
            else
                return 0
            end
            """;

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
            var archivedCount = 0;
            while (!cancellationToken.IsCancellationRequested)
            {
                var count = await ArchiveBatchAsync(dbContext, logService, cancellationToken);
                archivedCount += count;
                if (count < BatchSize)
                    break;
            }

            await logService.WriteAsync("marketplace_archive_success", new
            {
                archived_count = archivedCount
            });
        }

        // Insert and delete only the pinned ids: abort-list treats a listing missing from both tables as never created.
        private async Task<int> ArchiveBatchAsync(
            Marketplace.Service.DbContext dbContext,
            LogService logService,
            CancellationToken cancellationToken)
        {
            await using var conn = dbContext.GetUnifiedConnection();
            await conn.OpenAsync(cancellationToken);
            await using var transaction = await conn.BeginTransactionAsync(cancellationToken);

            try
            {
                var selectSql = $@"
                    SELECT `id` FROM `marketplace_listing`
                    WHERE `status` != {ListingState.ACTIVE.Escape()}
                        AND `updated_date` < NOW() - INTERVAL {(long)Retention.TotalSeconds} SECOND
                    ORDER BY `updated_date`
                    LIMIT {BatchSize}
                    FOR UPDATE";

                var ids = (await conn.QueryAsync<string>(selectSql, null, transaction)).ToList();
                if (ids.Count == 0)
                {
                    await transaction.CommitAsync(cancellationToken);
                    return 0;
                }

                var archiveSql = @"
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
                    WHERE `id` IN @Ids";

                var archivedCount = await conn.ExecuteAsync(archiveSql, new { Ids = ids }, transaction);
                var deletedCount = await conn.ExecuteAsync("DELETE FROM `marketplace_listing` WHERE `id` IN @Ids", new { Ids = ids }, transaction);
                if (archivedCount != ids.Count || deletedCount != ids.Count)
                    throw new InvalidOperationException($"Archive count mismatch: pinned {ids.Count}, archived {archivedCount}, deleted {deletedCount}");

                await transaction.CommitAsync(cancellationToken);
                return ids.Count;
            }
            catch (Exception ex)
            {
                await transaction.RollbackAsync(cancellationToken);
                _logger.LogError(ex, "Error archiving marketplace listings");
                await logService.WriteAsync("marketplace_archive_failed", new
                {
                    error = ex.Message
                });
                throw;
            }
        }
    }
}