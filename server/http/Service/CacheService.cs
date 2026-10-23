using Http.Redis;
using Http.Redis.Key;
using StackExchange.Redis;

namespace Http.Service
{
    public class CacheService
    {
        private const int DeleteBatchSize = 500;

        private static readonly string DeleteSettledKeysScript = """
            local COUNT_REFS = KEYS[#KEYS]
            local deleted = 0
            for i = 1, #KEYS - 1 do
                if redis.call('hexists', COUNT_REFS, KEYS[i]) == 0 then
                    deleted = deleted + redis.call('del', KEYS[i])
                end
            end
            return deleted
            """;

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

        private async Task<int> DeleteSettledCache(uint world, int index)
        {
            var redis = _redisService.GetDataConnection(world, index);
            if (redis == null)
            {
                return 0;
            }

            var keys = await redis.Connection.ScanKeysAsync("fb:cache:*", 1000);
            var count = 0;
            foreach (var chunk in keys.Chunk(DeleteBatchSize))
            {
                var scriptKeys = chunk
                    .Select(x => new RedisKey(x))
                    .Append(new RedisKey(Const.ReferenceCountKey))
                    .ToArray();
                count += (int)await redis.EvalAsync(DeleteSettledKeysScript, scriptKeys);
            }

            return count;
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
                var worldCount = 0;
                for (var i = -1; i < _redisService.GetShardSize(world); i++)
                {
                    worldCount += await DeleteSettledCache(world, i);
                }
                count += worldCount;
                _logger.LogInformation("Cache cleared for world {World}: {Count} keys deleted", world, worldCount);
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
            {
                return false;
            }

            var sessionKey = new SessionKey().Key;
            await redis.Connection.KeyDeleteAsync(new RedisKey(sessionKey));
            _logger.LogInformation("User sessions cleared");
            return true;
        }
    }
}
