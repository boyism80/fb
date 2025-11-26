using Http.Redis;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    public abstract class GenericCacheService<T> where T : class
    {
        protected readonly RedisService RedisService;

        protected readonly ILogger Logger;

        protected virtual TimeSpan DefaultTtl => TimeSpan.FromHours(1);

        protected GenericCacheService(
            RedisService redisService,
            ILogger logger)
        {
            RedisService = redisService;
            Logger = logger;
        }

        protected abstract string GenerateCacheKey(params object[] parameters);

        protected virtual uint? GetHashForSharding(string cacheKey, params object[] parameters)
        {
            return null;
        }

        public async Task<T> GetAsync(Func<Task<T>> dbQueryFunc, TimeSpan? ttl = null, params object[] parameters)
        {
            var cacheKey = GenerateCacheKey(parameters);
            var hashForSharding = GetHashForSharding(cacheKey, parameters);

            try
            {
                // Get Redis instance based on sharding
                var redis = hashForSharding.HasValue
                    ? RedisService.Redis(hashForSharding.Value)
                    : RedisService.Redis(cacheKey);

                if (redis == null)
                {
                    Logger.LogWarning($"Redis instance not available for key: {cacheKey}");
                    return await dbQueryFunc();
                }

                // Try to get from Redis cache
                var cachedValue = await redis.Connection.JsonGetAsync<T>(cacheKey);
                if (cachedValue != null)
                {
                    // Cache hit: refresh TTL
                    var effectiveTtl = ttl ?? DefaultTtl;
                    await redis.Connection.KeyExpireAsync(cacheKey, effectiveTtl);
                    return cachedValue;
                }

                // Cache miss: query database
                var dbValue = await dbQueryFunc();
                if (dbValue != null)
                {
                    // Store in Redis with TTL
                    var effectiveTtl = ttl ?? DefaultTtl;
                    await redis.Connection.JsonSetAsync(cacheKey, dbValue, effectiveTtl);
                }

                return dbValue;
            }
            catch (Exception ex)
            {
                Logger.LogError(ex, $"Error retrieving cache for key: {cacheKey}");
                // Fallback to database query on error
                return await dbQueryFunc();
            }
        }

        public async Task SetAsync(T value, TimeSpan? ttl = null, params object[] parameters)
        {
            var cacheKey = GenerateCacheKey(parameters);
            var hashForSharding = GetHashForSharding(cacheKey, parameters);

            try
            {
                // Get Redis instance based on sharding
                var redis = hashForSharding.HasValue
                    ? RedisService.Redis(hashForSharding.Value)
                    : RedisService.Redis(cacheKey);

                if (redis == null)
                {
                    Logger.LogWarning($"Redis instance not available for key: {cacheKey}");
                    return;
                }

                var effectiveTtl = ttl ?? DefaultTtl;
                await redis.Connection.JsonSetAsync(cacheKey, value, effectiveTtl);
            }
            catch (Exception ex)
            {
                Logger.LogError(ex, $"Error setting cache for key: {cacheKey}");
            }
        }

        public async Task DeleteAsync(params object[] parameters)
        {
            var cacheKey = GenerateCacheKey(parameters);
            var hashForSharding = GetHashForSharding(cacheKey, parameters);

            try
            {
                // Get Redis instance based on sharding
                var redis = hashForSharding.HasValue
                    ? RedisService.Redis(hashForSharding.Value)
                    : RedisService.Redis(cacheKey);

                if (redis == null)
                {
                    Logger.LogWarning($"Redis instance not available for key: {cacheKey}");
                    return;
                }

                await redis.Connection.KeyDeleteAsync(cacheKey);
            }
            catch (Exception ex)
            {
                Logger.LogError(ex, $"Error deleting cache for key: {cacheKey}");
            }
        }

        public async Task SetBatchAsync(Dictionary<object[], T> items, TimeSpan? ttl = null)
        {
            if (items == null || items.Count == 0)
                return;

            try
            {
                var effectiveTtl = ttl ?? DefaultTtl;

                // Group items by Redis shard
                var shardGroups = new Dictionary<Redis, Dictionary<string, T>>();
                foreach (var (parameters, value) in items)
                {
                    var cacheKey = GenerateCacheKey(parameters);
                    var hashForSharding = GetHashForSharding(cacheKey, parameters);

                    var redis = hashForSharding.HasValue
                        ? RedisService.Redis(hashForSharding.Value)
                        : RedisService.Redis(cacheKey);

                    if (redis == null)
                    {
                        Logger.LogWarning($"Redis instance not available for key: {cacheKey}");
                        continue;
                    }

                    if (!shardGroups.TryGetValue(redis, out var group))
                    {
                        group = new Dictionary<string, T>();
                        shardGroups[redis] = group;
                    }

                    group[cacheKey] = value;
                }

                // Set values for each shard
                var tasks = shardGroups.Select(async kvp =>
                {
                    var redis = kvp.Key;
                    var group = kvp.Value;

                    foreach (var (key, value) in group)
                    {
                        await redis.Connection.JsonSetAsync(key, value, effectiveTtl);
                    }
                });

                await Task.WhenAll(tasks);
            }
            catch (Exception ex)
            {
                Logger.LogError(ex, $"Error setting batch cache for {items.Count} items");
            }
        }

        public async Task DeleteBatchAsync(IEnumerable<object[]> parametersList)
        {
            if (parametersList == null)
                return;

            var parametersArray = parametersList.ToList();
            if (parametersArray.Count == 0)
                return;

            try
            {
                // Group keys by Redis shard
                var shardGroups = new Dictionary<Redis, List<string>>();
                foreach (var parameters in parametersArray)
                {
                    var cacheKey = GenerateCacheKey(parameters);
                    var hashForSharding = GetHashForSharding(cacheKey, parameters);

                    var redis = hashForSharding.HasValue
                        ? RedisService.Redis(hashForSharding.Value)
                        : RedisService.Redis(cacheKey);

                    if (redis == null)
                    {
                        Logger.LogWarning($"Redis instance not available for key: {cacheKey}");
                        continue;
                    }

                    if (!shardGroups.TryGetValue(redis, out var group))
                    {
                        group = new List<string>();
                        shardGroups[redis] = group;
                    }

                    group.Add(cacheKey);
                }

                // Delete keys for each shard
                var tasks = shardGroups.Select(async kvp =>
                {
                    var redis = kvp.Key;
                    var keys = kvp.Value;

                    foreach (var key in keys)
                    {
                        await redis.Connection.KeyDeleteAsync(key);
                    }
                });

                await Task.WhenAll(tasks);
            }
            catch (Exception ex)
            {
                Logger.LogError(ex, $"Error deleting batch cache for {parametersArray.Count} keys");
            }
        }
    }
}

