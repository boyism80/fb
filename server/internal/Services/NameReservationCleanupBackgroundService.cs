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
                WHERE `reserved_at` IS NOT NULL AND `reserved_at` < UTC_TIMESTAMP(3) - INTERVAL @threshold SECOND
                LIMIT @limit
                """,
                new { threshold = ReservationThresholdSeconds, limit = RowLimit })).ToList();

            var deleted = 0;
            foreach (var id in ids)
            {
                await using var transaction = await conn.BeginTransactionAsync();
                var world = await conn.QuerySingleOrDefaultAsync<uint?>(
                    """
                    SELECT `world` FROM `name_registry`
                    WHERE `id` = @id AND `reserved_at` IS NOT NULL AND `reserved_at` < UTC_TIMESTAMP(3) - INTERVAL @threshold SECOND
                    FOR UPDATE
                    """,
                    new { id, threshold = ReservationThresholdSeconds },
                    transaction);
                if (world == null)
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

            return (ids.Count, deleted);
        }
    }
}
