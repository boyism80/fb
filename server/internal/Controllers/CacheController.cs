using Http.Redis;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using StackExchange.Redis;

namespace Internal.Controllers
{
    /// <summary>
    /// Provides cache management operations for the internal API.
    /// Handles Redis cache clearing and maintenance tasks.
    /// </summary>
    [ApiController]
    [Route("cache")]
    public class CacheController : ControllerBase
    {
        private readonly RedisService _redisService;
        private readonly ILogger _logger;

        /// <summary>
        /// Initializes a new instance of the <see cref="CacheController"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="logger">The logger for recording cache operations.</param>
        public CacheController(RedisService redisService,
            ILogger<CacheController> logger)
        {
            _redisService = redisService;
            _logger = logger;
        }

        /// <summary>
        /// Deletes cache entries matching a specific pattern from a Redis shard.
        /// Scans for keys matching the pattern and deletes them in batches.
        /// </summary>
        /// <param name="i">The Redis shard index to operate on.</param>
        /// <param name="pattern">The key pattern to match for deletion.</param>
        /// <param name="count">The maximum number of keys to scan in one operation.</param>
        /// <returns>The number of keys that were deleted.</returns>
        private async Task<int> DeleteCache(int i, string pattern, int count)
        {
            var redis = _redisService.Redis(i);
            var conn = redis.Connection;
            var keys = await conn.ScanKeysAsync(pattern, 10);
            if (keys.Count > 0)
                await conn.KeyDeleteAsync(keys.Select(x => new RedisKey(x)).ToArray());

            return keys.Count;
        }

        /// <summary>
        /// Clears all cache data across all Redis shards.
        /// Removes cache entries, reference counts, and buffer keys from all shards.
        /// </summary>
        /// <returns>A task representing the asynchronous cache clearing operation.</returns>
        [HttpPost("clear")]
        public async Task Clear()
        {
            var count = 0;
            var batch = 10;
            for (int i = 0; i < _redisService.ShardSize; i++)
            {
                count += await DeleteCache(i, "cache:*", batch);
                count += await DeleteCache(i, Const.ReferenceCountKey, 10);
                count += await DeleteCache(i, $"{Const.RedisBufferKey}:*", 10);
            }
            _logger.LogInformation($"Cache cleared: {count} keys deleted");
        }
    }
}