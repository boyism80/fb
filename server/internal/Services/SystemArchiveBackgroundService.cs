using Dapper;
using Http.Service;

namespace Internal.Services
{
    // Every internal pod runs this: archiving and purging are idempotent, so overlapping runs are harmless.
    public class SystemArchiveBackgroundService : BackgroundService
    {
        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(1);
        private static readonly TimeSpan SweepInterval = TimeSpan.FromDays(1);

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

            var recipients = await dbContext.SystemStorageBox.GetRecipients(world);
            foreach (var chunk in recipients.Select(x => x.User).Distinct().Chunk(500))
            {
                var characters = await dbContext.Character.GetMany(world, chunk.ToList());
                var storageBoxes = await dbContext.StorageBox.GetMany(world, chunk.ToList());
                var finished = new List<Http.Model.SystemStorageBox>();
                foreach (var user in chunk)
                {
                    var taken = (storageBoxes.GetValueOrDefault(user) ?? Array.Empty<Http.Model.StorageBox>())
                        .Where(x => x.SystemStorageBoxId != null)
                        .Select(x => x.SystemStorageBoxId.Value)
                        .ToHashSet();
                    var exists = characters.ContainsKey(user);
                    finished.AddRange((await dbContext.SystemStorageBox.GetAll(world, user))
                        .Where(b => b.User == user)
                        .Where(b => exists == false || b.Canceled || (b.ExpiredDate != null && b.ExpiredDate <= now) || taken.Contains(b.Id)));
                }

                if (finished.Count > 0)
                {
                    await dbContext.SystemStorageBox.Archive(world, finished);
                    _logger.LogInformation("Archived {Count} recipient system storage boxes for world {World}", finished.Count, world);
                }
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
