using Http.Model;
using Newtonsoft.Json;

namespace Http.Service
{
    public class MaintenanceCacheService
    {
        private readonly RedisService _redisService;
        private readonly MaintenanceService _maintenanceService;
        private readonly ILogger<MaintenanceCacheService> _logger;
        private static readonly TimeSpan CacheTtl = TimeSpan.FromSeconds(60);

        public MaintenanceCacheService(
            RedisService redisService,
            MaintenanceService maintenanceService,
            ILogger<MaintenanceCacheService> logger)
        {
            _redisService = redisService;
            _maintenanceService = maintenanceService;
            _logger = logger;
        }

        public async Task<MaintenanceStatus> GetMaintenanceStatus(uint world)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return null;

            var cacheKey = $"fb:maintenance:status:{world}";
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

        public async Task InvalidateCache(uint world)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return;

            var cacheKey = $"fb:maintenance:status:{world}";
            await redis.Connection.KeyDeleteAsync(cacheKey);
        }
    }
}