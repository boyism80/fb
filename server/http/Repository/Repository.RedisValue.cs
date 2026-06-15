using Http.Model;
using Http.Reepository.Cache;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository
{
    public abstract class RedisValueRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisValueKey
    {
        private readonly LocalValueMemoryCacheLayer<TModel> _local;
        private readonly RedisValueCacheLayer<TModel> _redis;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly WriteBackService _dbExecuteService;

        protected RedisValueRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext)
        {
            _local = new LocalValueMemoryCacheLayer<TModel>();
            _redis = new RedisValueCacheLayer<TModel>(redisService);
            _distributedLock = distributedLock;
            _dbExecuteService = dbExecuteService;
        }

        private static string GetLockKey(TKey key) => $"fb:lock:{key.GetRedisKey()}";

        private static TModel AsActive(TModel value) =>
            value == null || value.Deleted ? null : value;

        protected override async Task<TModel> Get(uint world, TKey key)
        {
            await using var _ = await _distributedLock.Lock(world, GetLockKey(key));

            if (_local.ContainsKey(key.GetRedisKey()))
                return AsActive(_local.TryGet(key.GetRedisKey()));

            var redis = _redis.GetConnection(world, key.GetHash());
            var redisValue = await _redis.TryGetAsync(redis, key.GetRedisKey());
            if (redisValue != null)
                return AsActive(redisValue);

            var dbValue = await base.Get(world, key);
            if (dbValue == null)
                return null;

            await _redis.WriteBackAsync(redis, key.GetRedisKey(), dbValue);
            return dbValue;
        }

        protected override sealed Task<IEnumerable<TModel>> GetAll(uint world, TKey key)
        {
            throw new InvalidOperationException();
        }

        protected virtual async Task<IReadOnlyList<TModel>> GetMany(uint world, IReadOnlyList<TKey> keys)
        {
            if (keys == null || keys.Count == 0)
                return Array.Empty<TModel>();

            var result = new List<TModel>();
            var missingKeys = new List<TKey>();

            foreach (var group in keys.Distinct().GroupBy(k => k.GetHash()))
            {
                var redis = _redis.GetConnection(world, group.Key);
                if (redis == null)
                {
                    missingKeys.AddRange(group);
                    continue;
                }

                var keyList = group.ToList();
                var localMiss = new List<TKey>();
                foreach (var key in keyList)
                {
                    if (_local.ContainsKey(key.GetRedisKey()))
                    {
                        var localValue = AsActive(_local.TryGet(key.GetRedisKey()));
                        if (localValue != null)
                            result.Add(localValue);
                    }
                    else
                    {
                        localMiss.Add(key);
                    }
                }

                if (localMiss.Count > 0)
                {
                    var redisKeys = localMiss.Select(k => k.GetRedisKey()).ToList();
                    var redisHits = await _redis.TryGetManyAsync(redis, redisKeys);
                    var hitKeys = new HashSet<string>(redisHits.Select(x => x.RedisKey.ToString()));

                    foreach (var (redisKey, json) in redisHits)
                    {
                        _local.PutRaw(redisKey, json);

                        var val = JsonConvert.DeserializeObject<TModel>(json);
                        if (val == null)
                            continue;

                        var active = AsActive(val);
                        if (active != null)
                            result.Add(active);
                    }

                    foreach (var key in localMiss)
                    {
                        if (!hitKeys.Contains(key.GetRedisKey().ToString()))
                            missingKeys.Add(key);
                    }
                }
            }

            if (missingKeys.Count > 0)
            {
                var dbResult = await GetManyFromDb(world, missingKeys);
                foreach (var kv in dbResult)
                {
                    foreach (var entity in kv.Value)
                    {
                        result.Add(entity);
                    }

                    if (kv.Value.Count == 0)
                        continue;

                    var key = kv.Key;
                    var firstEntity = kv.Value[0];
                    await using var _ = await _distributedLock.Lock(world, GetLockKey(key));

                    var redis = _redis.GetConnection(world, key.GetHash());
                    await _redis.WriteBackAsync(redis, key.GetRedisKey(), firstEntity);
                    _local.Put(key.GetRedisKey(), firstEntity);
                }
            }

            return result;
        }

        public override TModel Set(uint world, TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                value.UpdatedDate = DateTime.Now;

                var redis = _redis.GetConnection(world, value.GetHash());
                if (redis == null)
                    return;

                await _redis.SetAsync(redis, value.GetRedisKey(), value);
                _local.Put(value.GetRedisKey(), value);

                var sql = OnUpsert(value);
                await _dbExecuteService.Post(world, value.GetHash(), sql, value.GetRedisKey().ToString());
            });
            return value;
        }

        public override sealed TModel[] Set(uint world, TModel[] values)
        {
            throw new InvalidOperationException();
        }
    }
}
