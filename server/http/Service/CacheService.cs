using Http.Redis;
using Http.Redis.Key;
using StackExchange.Redis;

namespace Http.Service
{
    /// <summary>
    /// Provides cache management operations for Redis.
    /// Handles cache clearing and maintenance tasks across all Redis shards.
    /// </summary>
    public class CacheService
    {
        private readonly RedisService _redisService;
        private readonly ILogger<CacheService> _logger;
        private readonly IConfiguration _configuration;
        private readonly ServerStateService _serverStateService;

        /// <summary>
        /// Initializes a new instance of the <see cref="CacheService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="logger">The logger for recording cache operations.</param>
        /// <param name="serverStateService">The server state service for checking running servers (optional).</param>
        public CacheService(RedisService redisService, ILogger<CacheService> logger, IConfiguration configuration, ServerStateService serverStateService)
        {
            _redisService = redisService;
            _logger = logger;
            _configuration = configuration;
            _serverStateService = serverStateService;
        }

        /// <summary>
        /// Deletes cache entries matching a specific pattern from a Redis shard.
        /// Scans for keys matching the pattern and deletes them in batches.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified.</param>
        /// <param name="shardIndex">The Redis shard index to operate on.</param>
        /// <param name="pattern">The key pattern to match for deletion.</param>
        /// <param name="count">The maximum number of keys to scan in one operation.</param>
        /// <returns>The number of keys that were deleted.</returns>
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

        /// <summary>
        /// Clears all cache data across all Redis shards.
        /// Removes cache entries, reference counts, and buffer keys from all shards.
        /// </summary>
        /// <returns>The total number of keys that were deleted across all shards.</returns>
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

        /// <summary>
        /// Clears all user sessions from Redis.
        /// This operation is only safe when no servers are running.
        /// </summary>
        /// <returns>True if sessions were cleared; false if servers are still running or ServerStateService is not available.</returns>
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

