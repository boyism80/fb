using Http.Model;
using Http.Redis;
using Http.Service;
using StackExchange.Redis;

namespace Internal.Services
{
    /// <summary>
    /// Provides a background service that periodically monitors maintenance schedules and triggers force logout.
    /// Uses Redis distributed locking to ensure only one instance executes across all internal server instances.
    /// </summary>
    public class MaintenanceBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<MaintenanceBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromSeconds(30);
        private static readonly TimeSpan RetryInterval = TimeSpan.FromSeconds(10);
        private const string LockKey = "maintenance:background:lock";
        private const int LockTtlSeconds = 60; // Longer than processing interval to prevent overlap

        /// <summary>
        /// Initializes a new instance of the <see cref="MaintenanceBackgroundService"/> class.
        /// </summary>
        public MaintenanceBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            ILogger<MaintenanceBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _redisService = redisService;
            _logger = logger;
        }

        /// <summary>
        /// Executes the background service that periodically monitors maintenance schedules.
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

        /// <summary>
        /// Attempts to acquire the distributed lock using Lua script.
        /// </summary>
        /// <param name="redis">The Redis connection.</param>
        /// <returns>True if lock was acquired; otherwise, false.</returns>
        private async Task<bool> TryAcquireLockAsync(Http.Service.Redis redis)
        {
            try
            {
                var result = await redis.ScriptEvaluateAsync("acquire_maintenance_lock.lua", new
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

        /// <summary>
        /// Processes maintenance checks for all worlds and triggers force logout if needed.
        /// </summary>
        /// <param name="maintenanceService">The maintenance service.</param>
        /// <param name="cancellationToken">The cancellation token.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        private async Task ProcessMaintenanceChecksAsync(
            MaintenanceService maintenanceService,
            CancellationToken cancellationToken)
        {
            // Get all worlds from Redis keys (maintenance:index:*)
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return;

            var keys = await redis.Connection.ScanKeysAsync("maintenance:index:*", 1000);
            var worlds = new HashSet<uint>();
            var processedWorlds = new HashSet<uint>();

            foreach (var key in keys)
            {
                var keyStr = key.ToString();
                // Format: maintenance:index:{world}
                var parts = keyStr.Split(':');
                if (parts.Length == 3 && uint.TryParse(parts[2], out var world))
                {
                    worlds.Add(world);
                }
            }

            var now = DateTime.Now;
            var lastMaintenanceState = new Dictionary<uint, bool>();

            // Check each world for maintenance status
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
                    processedWorlds.Add(world);
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to process maintenance check for world {World}", world);
                }
            }

            // Clean up expired one-time schedules
            await CleanupExpiredSchedulesAsync(maintenanceService, processedWorlds, now, cancellationToken);
        }

        /// <summary>
        /// Cleans up expired one-time maintenance schedules.
        /// </summary>
        /// <param name="maintenanceService">The maintenance service.</param>
        /// <param name="worlds">The worlds to check.</param>
        /// <param name="now">The current time.</param>
        /// <param name="cancellationToken">The cancellation token.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        private async Task CleanupExpiredSchedulesAsync(
            MaintenanceService maintenanceService,
            HashSet<uint> worlds,
            DateTime now,
            CancellationToken cancellationToken)
        {
            foreach (var world in worlds)
            {
                if (cancellationToken.IsCancellationRequested)
                    break;

                try
                {
                    var schedules = await maintenanceService.GetAllSchedules(world);
                    foreach (var schedule in schedules)
                    {
                        if (cancellationToken.IsCancellationRequested)
                            break;

                        // Only clean up one-time schedules that have expired
                        if (schedule.RepeatType == MaintenanceRepeatType.None &&
                            schedule.IsActive &&
                            schedule.EndTime < now)
                        {
                            // Remove from active sorted set (already done by expiration, but ensure cleanup)
                            var redis = _redisService.GetUnifiedConnection();
                            if (redis != null)
                            {
                                var activeKey = $"maintenance:active:{world}";
                                await redis.Connection.SortedSetRemoveAsync(activeKey, schedule.Id);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to cleanup expired schedules for world {World}", world);
                }
            }
        }
    }
}
