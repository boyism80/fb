using Dapper;
using Http.Service;

namespace Internal.Services
{
    // Every internal pod runs this: archiving and purging are idempotent, so overlapping runs are harmless.
    public class SystemArchiveBackgroundService : BackgroundService
    {
        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(1);

        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<SystemArchiveBackgroundService> _logger;

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
                        break;

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
                // These mails are expired or canceled and never sent again, so their received records no longer
                // guard anything. They are purged before archiving: a failed purge leaves the mails in Redis to
                // retry, while archiving first would drop them from the next run.
                var ids = mails.Select(m => m.Id).ToArray();
                for (var db = 0; db < dbContext.GetShardDbSize(world); db++)
                {
                    await using var conn = dbContext.GetDataConnection(world, db);
                    await conn.ExecuteAsync("DELETE FROM `mail_system_received` WHERE `system_mail_id` IN @ids", new { ids });
                }

                await dbContext.SystemMail.Archive(world, mails);
                _logger.LogInformation("Archived {Count} system mails for world {World}", mails.Count, world);
            }

            var boxes = (await dbContext.SystemStorageBox.GetAll(world))
                .Where(b => b.Canceled || (b.ExpiredDate != null && b.ExpiredDate <= now))
                .ToList();
            if (boxes.Count > 0)
            {
                await dbContext.SystemStorageBox.Archive(world, boxes);
                _logger.LogInformation("Archived {Count} system storage boxes for world {World}", boxes.Count, world);
            }
        }
    }
}
