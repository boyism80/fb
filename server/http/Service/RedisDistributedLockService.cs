using Medallion.Threading.Redis;

namespace Http.Service
{
    /// <summary>
    /// Provides distributed locking functionality using Redis as the coordination mechanism.
    /// Enables thread-safe operations across multiple application instances in a distributed environment.
    /// </summary>
    public class RedisDistributedLockService
    {
        private readonly RedisService _redisService;

        /// <summary>
        /// Initializes a new instance of the <see cref="RedisDistributedLockService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for accessing Redis connections.</param>
        public RedisDistributedLockService(RedisService redisService)
        {
            _redisService = redisService;
        }

        /// <summary>
        /// Acquires a distributed lock for the specified key using Redis.
        /// The lock ensures mutual exclusion across all application instances that share the same Redis cluster.
        /// </summary>
        /// <param name="key">The unique key identifying the resource to lock.</param>
        /// <returns>A Redis distributed lock handle that must be disposed to release the lock.</returns>
        public async Task<RedisDistributedLockHandle> Lock(uint world, string key)
        {
            var redis = _redisService.GetShardConnection(world, key);
            if (redis == null)
                throw new Exception($"Redis instance not found for world {world} and key {key}");
            return await new RedisDistributedLock(key, redis.Connection).AcquireAsync();
        }
    }
}
