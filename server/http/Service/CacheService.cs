using Http.Redis;
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

        /// <summary>
        /// Initializes a new instance of the <see cref="CacheService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="logger">The logger for recording cache operations.</param>
        public CacheService(RedisService redisService, ILogger<CacheService> logger)
        {
            _redisService = redisService;
            _logger = logger;
        }

        /// <summary>
        /// Deletes cache entries matching a specific pattern from a Redis shard.
        /// Scans for keys matching the pattern and deletes them in batches.
        /// </summary>
        /// <param name="shardIndex">The Redis shard index to operate on.</param>
        /// <param name="pattern">The key pattern to match for deletion.</param>
        /// <param name="count">The maximum number of keys to scan in one operation.</param>
        /// <returns>The number of keys that were deleted.</returns>
        private async Task<int> DeleteCache(int shardIndex, string pattern, int count)
        {
            var redis = _redisService.Redis(shardIndex);
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
            var batch = 10;
            for (int i = 0; i < _redisService.ShardSize; i++)
            {
                count += await DeleteCache(i, "cache:*", batch);
                count += await DeleteCache(i, Const.ReferenceCountKey, 10);
                count += await DeleteCache(i, $"{Const.RedisBufferKey}:*", 10);
            }
            _logger.LogInformation("Cache cleared: {Count} keys deleted", count);
            return count;
        }
    }
}

