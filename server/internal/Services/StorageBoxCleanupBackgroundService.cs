using Http.Service;

namespace Internal.Services
{
    public class StorageBoxCleanupBackgroundService : BackgroundService
    {
        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(10);
        private const int UserLimit = 500;

        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<StorageBoxCleanupBackgroundService> _logger;

        public StorageBoxCleanupBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            ILogger<StorageBoxCleanupBackgroundService> logger)
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
                        using var scope = _scopeFactory.CreateScope();
                        var storageService = scope.ServiceProvider.GetRequiredService<StorageService>();
                        var deleted = await storageService.DeleteReceivedBoxesAsync(world, UserLimit);
                        if (deleted > 0)
                            _logger.LogInformation("Deleted {Count} received storage boxes for world {World}", deleted, world);
                    }
                    catch (Exception e)
                    {
                        _logger.LogWarning(e, "Failed to delete received storage boxes for world {World}", world);
                    }
                }

                await Task.Delay(ProcessingInterval, stoppingToken);
            }
        }
    }
}
