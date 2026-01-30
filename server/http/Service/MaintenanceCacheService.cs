using Http.Model;
using Http.Redis;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    /// <summary>
    /// Provides caching functionality for maintenance status checks.
    /// Reduces Redis queries by caching maintenance status with TTL.
    /// </summary>
    public class MaintenanceCacheService
    {
        private readonly RedisService _redisService;
        private readonly MaintenanceService _maintenanceService;
        private readonly ILogger<MaintenanceCacheService> _logger;
        private static readonly TimeSpan CacheTtl = TimeSpan.FromSeconds(60);

        /// <summary>
        /// Initializes a new instance of the <see cref="MaintenanceCacheService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for accessing Redis connections.</param>
        /// <param name="maintenanceService">The maintenance service for retrieving maintenance information.</param>
        /// <param name="logger">The logger for recording operations.</param>
        public MaintenanceCacheService(
            RedisService redisService,
            MaintenanceService maintenanceService,
            ILogger<MaintenanceCacheService> logger)
        {
            _redisService = redisService;
            _maintenanceService = maintenanceService;
            _logger = logger;
        }

        /// <summary>
        /// Gets the cached maintenance status for the specified world.
        /// If cache is expired or missing, refreshes from MaintenanceService.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>The maintenance status if found; otherwise, null.</returns>
        public async Task<MaintenanceStatus> GetMaintenanceStatus(uint world)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return null;

            var cacheKey = $"maintenance:status:{world}";
            var cachedValue = await redis.Connection.StringGetAsync(cacheKey);
            
            if (!cachedValue.IsNull)
            {
                try
                {
                    var status = JsonConvert.DeserializeObject<MaintenanceStatus>(cachedValue.ToString());
                    if (status != null && status.NextCheckTime.HasValue && status.NextCheckTime.Value > DateTime.UtcNow)
                    {
                        return status;
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to deserialize cached maintenance status for world {World}", world);
                }
            }

            // Cache miss or expired - refresh from MaintenanceService
            var maintenanceInfo = await _maintenanceService.GetMaintenanceInfo(world);
            var statusToCache = new MaintenanceStatus
            {
                IsActive = maintenanceInfo != null && maintenanceInfo.IsActive,
                CurrentMaintenance = maintenanceInfo,
                NextCheckTime = DateTime.UtcNow.Add(CacheTtl)
            };

            // Update cache
            try
            {
                var statusJson = JsonConvert.SerializeObject(statusToCache);
                await redis.Connection.StringSetAsync(cacheKey, statusJson, CacheTtl);
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to cache maintenance status for world {World}", world);
            }

            return statusToCache;
        }

        /// <summary>
        /// Invalidates the maintenance status cache for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        public async Task InvalidateCache(uint world)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return;

            var cacheKey = $"maintenance:status:{world}";
            await redis.Connection.KeyDeleteAsync(cacheKey);
        }
    }
}
