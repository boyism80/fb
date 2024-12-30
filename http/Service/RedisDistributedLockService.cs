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

        public async Task<RedisDistributedLockHandle> Lock(string key)
        {
            return await new RedisDistributedLock(key, _redisService.Redis(key).Connection).AcquireAsync();
        }
    }
}
