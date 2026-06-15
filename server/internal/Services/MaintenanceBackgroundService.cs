using Http.Redis;
using Http.Service;

namespace Internal.Services
{
    public class MaintenanceBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<MaintenanceBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromSeconds(10);
        private static readonly TimeSpan RetryInterval = TimeSpan.FromSeconds(1);
        private const string LockKey = "fb:maintenance:background:lock";
        private const int LockTtlSeconds = 60; // Longer than processing interval to prevent overlap

        private static readonly string AcquireMaintenanceLockScript = """
            if redis.call('exists', @lock_key) == 0 then
                redis.call('set', @lock_key, '1')
                redis.call('expire', @lock_key, @ttl_seconds)
                return 1
            else
                return 0
            end
            """;

        public MaintenanceBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            ILogger<MaintenanceBackgroundService> logger)
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
                    if (redis == null)
                    {
                        _logger.LogWarning("Redis unified connection not available, retrying in {Interval}", RetryInterval);
                        await Task.Delay(RetryInterval, stoppingToken);
                        continue;
                    }

                    var lockAcquired = await TryAcquireLockAsync(redis);

                    if (lockAcquired)
                    {
                        try
                        {
                            // Create scope for service resolution
                            using var scope = _scopeFactory.CreateScope();
                            var maintenanceService = scope.ServiceProvider.GetRequiredService<MaintenanceService>();

                            // Process maintenance checks for all worlds
                            await ProcessMaintenanceChecksAsync(maintenanceService, stoppingToken);
                        }
                        catch (Exception e)
                        {
                            _logger.LogError(e, "Error processing maintenance checks: {Message}", e.Message);
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
                    _logger.LogError(e, "Error in maintenance background service: {Message}", e.Message);

                    // Wait on error as well
                    await Task.Delay(RetryInterval, stoppingToken);
                }
            }
        }

        private async Task<bool> TryAcquireLockAsync(Http.Service.Redis redis)
        {
            try
            {
                var result = await redis.EvalAsync(AcquireMaintenanceLockScript, new
                {
                    lock_key = LockKey,
                    ttl_seconds = LockTtlSeconds
                });
                return (int)result == 1;
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to acquire maintenance background lock");
                return false;
            }
        }

        private async Task ProcessMaintenanceChecksAsync(
            MaintenanceService maintenanceService,
            CancellationToken cancellationToken)
        {
            var worlds = _redisService.GetConfiguredWorlds();
            if (worlds.Count == 0)
                return;

            var now = DateTime.Now;
            var lastMaintenanceState = new Dictionary<uint, bool>();

            foreach (var world in worlds)
            {
                if (cancellationToken.IsCancellationRequested)
                    break;

                try
                {
                    var wasInMaintenance = lastMaintenanceState.GetValueOrDefault(world, false);
                    var maintenanceInfo = await maintenanceService.GetMaintenanceInfo(world, now);
                    var isInMaintenance = maintenanceInfo != null && maintenanceInfo.IsActive;

                    // If maintenance just started (wasn't active before, now is active), force logout
                    if (!wasInMaintenance && isInMaintenance)
                    {
                        _logger.LogInformation("Maintenance started for world {World}, forcing logout of regular users", world);
                        var kickedCount = await maintenanceService.ForceLogoutRegularUsers(world);
                        _logger.LogInformation("Kicked {Count} regular users from world {World} due to maintenance start", kickedCount, world);
                    }

                    lastMaintenanceState[world] = isInMaintenance;
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to process maintenance check for world {World}", world);
                }
            }

            // Remove expired one-time schedules from all configured worlds
            await CleanupExpiredSchedulesAsync(maintenanceService, worlds, now, cancellationToken);
        }

        private async Task CleanupExpiredSchedulesAsync(
            MaintenanceService maintenanceService,
            IReadOnlyList<uint> worlds,
            DateTime now,
            CancellationToken cancellationToken)
        {
            foreach (var world in worlds)
            {
                if (cancellationToken.IsCancellationRequested)
                    break;

                try
                {
                    await maintenanceService.RemoveExpiredSchedules(world, now);
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to cleanup expired schedules for world {World}", world);
                }
            }
        }
    }
}