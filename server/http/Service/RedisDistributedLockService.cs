using Medallion.Threading.Redis;

namespace Http.Service
{
    public class RedisDistributedLockService
    {
        private static readonly TimeSpan DefaultTimeout = TimeSpan.FromSeconds(10);

        private readonly RedisService _redisService;

        public RedisDistributedLockService(RedisService redisService)
        {
            _redisService = redisService;
        }

        public async Task<RedisDistributedLockHandle> Lock(uint world, string key, CancellationToken cancellationToken = default)
        {
            var redis = _redisService.GetShardConnection(world, key) ??
                throw new Exception($"Redis instance not found for world {world} and key {key}");

            return await new RedisDistributedLock(key, redis.Connection).AcquireAsync(DefaultTimeout, cancellationToken);
        }
    }
}