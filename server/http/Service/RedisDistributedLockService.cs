using Medallion.Threading.Redis;

namespace Http.Service
{
    public class RedisDistributedLockService
    {
        private readonly RedisService _redisService;

        public RedisDistributedLockService(RedisService redisService)
        {
            _redisService = redisService;
        }

        public async Task<RedisDistributedLockHandle> Lock(uint world, string key)
        {
            var redis = _redisService.GetShardConnection(world, key) ??
                throw new Exception($"Redis instance not found for world {world} and key {key}");

            return await new RedisDistributedLock(key, redis.Connection).AcquireAsync();
        }
    }
}