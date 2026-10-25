using Dapper;
using Http.Service;

namespace Internal.Services
{
    // Every internal pod runs this: archiving and purging are idempotent, so overlapping runs are harmless.
    public class SystemArchiveBackgroundService : BackgroundService
    {
        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(1);
        private static readonly TimeSpan SweepInterval = TimeSpan.FromDays(1);
        private static readonly TimeSpan ArchiveExpiryMargin = TimeSpan.FromDays(1);

        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<SystemArchiveBackgroundService> _logger;
        private readonly Dictionary<uint, DateTime> _lastSweeps = new();

        public SystemArchiveBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            ILogger<SystemArchiveBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _redisService = redisService;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                foreach (var world in _redisService.GetConfiguredWorlds())
                {
                    if (stoppingToken.IsCancellationRequested)
                    {
                        break;
                    }

                    try
                    {
                        await ArchiveAsync(world);
                    }
                    catch (Exception e)
                    {
                        _logger.LogWarning(e, "Failed to archive system mails and boxes for world {World}", world);
                    }
                }

                await Task.Delay(ProcessingInterval, stoppingToken);
            }
        }

        private async Task ArchiveAsync(uint world)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();
            var now = DateTime.UtcNow;

            var mails = (await dbContext.SystemMail.GetAll(world))
                .Where(m => m.Canceled || (m.ExpireDate != null && m.ExpireDate <= now))
                .ToList();
            if (mails.Count > 0)
            {
                await dbContext.SystemMail.Archive(world, mails);
                _logger.LogInformation("Archived {Count} system mails for world {World}", mails.Count, world);
            }

            // An archived mail is never sent again (send-system-mail rejects it), so its deleted mail rows and
            // received records guard nothing and are always safe to delete. This purge clears what exists when a
            // mail is archived; the daily sweep below clears whatever appears later. Deleted rows go first because
            // USP_MAIL_NEXT_ID turns them into received records. A failed purge leaves `purged` at 0 and runs again.
            await using (var global = dbContext.GetGlobalConnection(world))
            {
                var ids = (await global.QueryAsync<uint>(
                    "SELECT `id` FROM `system_mail_archive` WHERE `purged` = 0 LIMIT 500")).ToArray();
                if (ids.Length > 0)
                {
                    for (var db = 0; db < dbContext.GetShardDbSize(world); db++)
                    {
                        await using var conn = dbContext.GetDataConnection(world, db);
                        await conn.ExecuteAsync("DELETE FROM `mail` WHERE `system_mail_id` IN @ids AND `deleted` = 1", new { ids });
                        await conn.ExecuteAsync("DELETE FROM `mail_system_received` WHERE `system_mail_id` IN @ids", new { ids });
                    }

                    await global.ExecuteAsync("UPDATE `system_mail_archive` SET `purged` = 1 WHERE `id` IN @ids", new { ids });
                    _logger.LogInformation("Purged received records of {Count} archived system mails for world {World}", ids.Length, world);
                }
            }

            if (_lastSweeps.TryGetValue(world, out var lastSweep) == false || now - lastSweep >= SweepInterval)
            {
                await SweepArchivedMailRecordsAsync(dbContext, world);
                await SweepArchivedStorageBoxesAsync(dbContext, scope.ServiceProvider.GetRequiredService<RedisDistributedLockService>(), world);
                _lastSweeps[world] = now;
            }

            var boxes = (await dbContext.SystemStorageBox.GetAll(world))
                .Where(b => b.Canceled || (b.ExpiredDate != null && b.ExpiredDate <= now))
                .ToList();
            if (boxes.Count > 0)
            {
                await dbContext.SystemStorageBox.Archive(world, boxes);
                _logger.LogInformation("Archived {Count} system storage boxes for world {World}", boxes.Count, world);
            }

            var personal = await dbContext.SystemStorageBox.GetFinishedPersonal(world);
            if (personal.Count > 0)
            {
                await dbContext.SystemStorageBox.Archive(world, personal);
                _logger.LogInformation("Archived {Count} personal system storage boxes for world {World}", personal.Count, world);
            }
        }

        private async Task SweepArchivedStorageBoxesAsync(DbContext dbContext, RedisDistributedLockService distributedLock, uint world)
        {
            var deleted = 0;
            int count;
            do
            {
                count = await dbContext.SystemStorageBox.DeleteExpiredArchive(world, ArchiveExpiryMargin, 1000);
                deleted += count;
            } while (count == 1000);

            var afterId = 0u;
            List<(uint Id, string ExternalRef)> rows;
            do
            {
                rows = await dbContext.SystemStorageBox.GetArchivedMarketplace(world, afterId, 500);
                if (rows.Count == 0)
                {
                    break;
                }

                afterId = rows[^1].Id;
                var refs = rows.Select(x => x.ExternalRef).ToArray();
                HashSet<string> parcels;
                await using (var unified = dbContext.GetUnifiedConnection())
                {
                    parcels = (await unified.QueryAsync<string>(
                        "SELECT `external_ref` FROM `marketplace_parcel` WHERE `external_ref` IN @refs",
                        new { refs })).ToHashSet();
                }

                foreach (var row in rows.Where(x => parcels.Contains(x.ExternalRef) == false))
                {
                    await using var _ = await distributedLock.Lock(world, StorageService.ExternalRefLockKey(row.ExternalRef));
                    await dbContext.SystemStorageBox.DeleteArchived(world, row.Id);
                    deleted++;
                }
            } while (rows.Count == 500);

            if (deleted > 0)
            {
                _logger.LogInformation("Deleted {Count} archived system storage boxes for world {World}", deleted, world);
            }
        }

        // Finds archived ids from the rows themselves, so a row left by any path (a mail deleted after its system
        // mail was archived, a renumber, a failed request) is removed by the next sweep however it got there.
        private async Task SweepArchivedMailRecordsAsync(DbContext dbContext, uint world)
        {
            await using var global = dbContext.GetGlobalConnection(world);
            var swept = 0;
            for (var db = 0; db < dbContext.GetShardDbSize(world); db++)
            {
                await using var conn = dbContext.GetDataConnection(world, db);
                var ids = (await conn.QueryAsync<uint>(
                    """
                    SELECT DISTINCT `system_mail_id` FROM `mail` WHERE `deleted` = 1 AND `system_mail_id` IS NOT NULL
                    UNION
                    SELECT DISTINCT `system_mail_id` FROM `mail_system_received`
                    """)).ToArray();

                foreach (var chunk in ids.Chunk(1000))
                {
                    var archived = (await global.QueryAsync<uint>(
                        "SELECT `id` FROM `system_mail_archive` WHERE `id` IN @chunk", new { chunk })).ToArray();
                    if (archived.Length == 0)
                    {
                        continue;
                    }

                    await conn.ExecuteAsync("DELETE FROM `mail` WHERE `system_mail_id` IN @archived AND `deleted` = 1", new { archived });
                    await conn.ExecuteAsync("DELETE FROM `mail_system_received` WHERE `system_mail_id` IN @archived", new { archived });
                    swept += archived.Length;
                }
            }

            if (swept > 0)
            {
                _logger.LogInformation("Swept records of {Count} archived system mail ids for world {World}", swept, world);
            }
        }
    }
}
