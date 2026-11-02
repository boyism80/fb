using Dapper;
using Http.Model;
using Http.Service;

namespace Internal.Services
{
    public class NameReservationCleanupBackgroundService : BackgroundService
    {
        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(5);
        private const int ReservationThresholdSeconds = 120;
        private const int RowLimit = 500;

        private readonly IServiceScopeFactory _scopeFactory;
        private readonly ILogger<NameReservationCleanupBackgroundService> _logger;
        private uint _after;

        public NameReservationCleanupBackgroundService(
            IServiceScopeFactory scopeFactory,
            ILogger<NameReservationCleanupBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                var full = false;
                try
                {
                    var (found, deleted) = await DeleteExpiredAsync();
                    full = found >= RowLimit;
                    if (deleted > 0)
                    {
                        _logger.LogInformation("Deleted {Count} expired name reservations", deleted);
                    }
                }
                catch (Exception e)
                {
                    _logger.LogWarning(e, "Failed to delete expired name reservations");
                }

                if (full == false)
                {
                    await Task.Delay(ProcessingInterval, stoppingToken);
                }
            }
        }

        private async Task<(int Found, int Deleted)> DeleteExpiredAsync()
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.GetUnifiedConnection();
            await conn.OpenAsync();
            var ids = (await conn.QueryAsync<uint>(
                """
                SELECT `id` FROM `name_registry`
                WHERE `reserved_at` IS NOT NULL AND `reserved_at` < UTC_TIMESTAMP(3) - INTERVAL @threshold SECOND AND `id` > @after
                ORDER BY `id`
                LIMIT @limit
                """,
                new { threshold = ReservationThresholdSeconds, after = _after, limit = RowLimit })).ToList();
            _after = ids.Count >= RowLimit ? ids[^1] : 0;

            var deleted = 0;
            var failedWorlds = new HashSet<uint>();
            foreach (var id in ids)
            {
                var world = (uint?)null;
                try
                {
                    await using var transaction = await conn.BeginTransactionAsync();
                    world = await conn.QuerySingleOrDefaultAsync<uint?>(
                        """
                        SELECT `world` FROM `name_registry`
                        WHERE `id` = @id AND `reserved_at` IS NOT NULL AND `reserved_at` < UTC_TIMESTAMP(3) - INTERVAL @threshold SECOND
                        FOR UPDATE
                        """,
                        new { id, threshold = ReservationThresholdSeconds },
                        transaction);
                    if (world == null || failedWorlds.Contains(world.Value))
                    {
                        await transaction.RollbackAsync();
                        continue;
                    }

                    if (world.Value != 0)
                    {
                        dbContext.Character.Delete(world.Value, new CharacterKey { Id = id });
                        await dbContext.SaveChangesAsync();
                    }

                    await conn.ExecuteAsync("DELETE FROM `name_registry` WHERE `id` = @id", new { id }, transaction);
                    await transaction.CommitAsync();
                    deleted++;
                }
                catch (Exception e) when (world != null)
                {
                    failedWorlds.Add(world.Value);
                    _logger.LogWarning(e, "Failed to delete expired name reservation {Id} in world {World}", id, world.Value);
                }
            }

            return (ids.Count, deleted);
        }
    }
}
