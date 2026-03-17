using Http.Redis;
using Http.Redis.Key;
using StackExchange.Redis;

namespace Http.Service
{
    public class CacheService
    {
        private readonly RedisService _redisService;
        private readonly ILogger<CacheService> _logger;
        private readonly IConfiguration _configuration;
        private readonly ServerStateService _serverStateService;

        public CacheService(RedisService redisService, ILogger<CacheService> logger, IConfiguration configuration, ServerStateService serverStateService)
        {
            _redisService = redisService;
            _logger = logger;
            _configuration = configuration;
            _serverStateService = serverStateService;
        }

        private async Task<int> DeleteCache(uint world, int shardIndex, string pattern, int count)
        {
            var redis = _redisService.GetDataConnection(world, shardIndex);
            if (redis == null)
                return 0;

            var conn = redis.Connection;
            var keys = await conn.ScanKeysAsync(pattern, count);
            if (keys.Count > 0)
                await conn.KeyDeleteAsync(keys.Select(x => new RedisKey(x)).ToArray());

            return keys.Count;
        }

        public async Task<int> ClearCache()
        {
            var count = 0;
            // Get all configured worlds from Redis:worlds section
            var worldsSection = _configuration.GetSection("Redis:worlds");
            var worlds = worldsSection.GetChildren()
                .Where(child => uint.TryParse(child.Key, out _))
                .Select(child => uint.Parse(child.Key));

            foreach (var world in worlds)
            {
                var batch = 10;
                for (int i = 0; i < _redisService.GetShardSize(world); i++)
                {
                    count += await DeleteCache(world, i, "cache:*", batch);
                    count += await DeleteCache(world, i, Const.ReferenceCountKey, 10);
                    count += await DeleteCache(world, i, $"{Const.RedisBufferKey}:*", 10);
                }
                _logger.LogInformation("Cache cleared for world {World}: {Count} keys deleted", world, count);
            }
            return count;
        }

        public async Task<bool> ClearUserSessions(uint world)
        {
            if (_serverStateService == null)
            {
                _logger.LogWarning("Cannot clear user sessions: ServerStateService is not available");
                return false;
            }

            var hasRunning = await _serverStateService.HasRunningServers();
            if (hasRunning)
            {
                _logger.LogWarning("Cannot clear user sessions: servers are still running");
                return false;
            }

            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return false;
            var sessionKey = new SessionKey().Key;
            await redis.Connection.KeyDeleteAsync(new RedisKey(sessionKey));
            _logger.LogInformation("User sessions cleared");
            return true;
        }
    }
}
