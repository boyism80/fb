using Http.Redis;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using StackExchange.Redis;

namespace Internal.Controllers
{
    [ApiController]
    [Route("cache")]
    public class CacheController : ControllerBase
    {
        private readonly RedisService _redisService;
        private readonly ILogger _logger;

        public CacheController(RedisService redisService,
            ILogger<CacheController> logger)
        {
            _redisService = redisService;
            _logger = logger;
        }

        private async Task<int> DeleteCache(int i, string pattern, int count)
        {
            var redis = _redisService.Redis(i);
            var conn = redis.Connection;
            var keys = await conn.ScanKeysAsync(pattern, "10");
            if (keys.Count > 0)
                await conn.KeyDeleteAsync(keys.Select(x => new RedisKey(x)).ToArray());

            return keys.Count;
        }

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