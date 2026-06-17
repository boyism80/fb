using Http.Model;
using Http.Reepository.Cache;
using Http.Service;
using Newtonsoft.Json;

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

        protected override async Task<TModel> Get(uint world, TKey key)
        {
            await using var _ = await _distributedLock.Lock(world, GetLockKey(key));

            if (_local.ContainsKey(key.GetRedisKey()))
                return _local.TryGet(key.GetRedisKey());

            var redis = _redis.GetConnection(world, key.GetHash());
            var (status, redisValue) = await _redis.LookupAsync(redis, key.GetRedisKey());
            if (status == RedisCacheLookupStatus.Pending)
                return null;

            if (status == RedisCacheLookupStatus.Hit)
            {
                _local.Put(key.GetRedisKey(), redisValue);
                return redisValue;
            }

            var dbValue = await base.Get(world, key);
            if (dbValue == null)
                return null;

            if (await _redis.WriteBackAsync(redis, key.GetRedisKey(), dbValue))
                _local.Put(key.GetRedisKey(), dbValue);

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
                        result.Add(_local.TryGet(key.GetRedisKey()));
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
                        if (val != null)
                            result.Add(val);
                    }

                    foreach (var key in localMiss)
                    {
                        if (hitKeys.Contains(key.GetRedisKey().ToString()))
                            continue;

                        var (status, _) = await _redis.LookupAsync(redis, key.GetRedisKey());
                        if (status != RedisCacheLookupStatus.Pending)
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
                    if (await _redis.WriteBackAsync(redis, key.GetRedisKey(), firstEntity))
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

        public override void Delete(uint world, TKey key)
        {
            _buffer.Enqueue(async () =>
            {
                var redisKey = key.GetRedisKey();
                var redis = _redis.GetConnection(world, key.GetHash());
                if (redis == null)
                    return;

                await using (await _distributedLock.Lock(world, GetLockKey(key)))
                {
                    await _redis.RemovePendingWriteBackAsync(redis, redisKey);
                    _local.Remove(redisKey);
                }

                var sql = OnDelete(key);
                await _dbExecuteService.Post(world, key.GetHash(), sql, redisKey.ToString());
            });
        }

        public override void Delete(uint world, TKey[] keys)
        {
            if (keys == null || keys.Length == 0)
                return;

            _buffer.Enqueue(async () =>
            {
                foreach (var hashGroup in keys.GroupBy(k => k.GetHash()))
                {
                    var redis = _redis.GetConnection(world, hashGroup.Key);
                    if (redis == null)
                        continue;

                    foreach (var key in hashGroup)
                    {
                        var redisKey = key.GetRedisKey();

                        await using (await _distributedLock.Lock(world, GetLockKey(key)))
                        {
                            await _redis.RemovePendingWriteBackAsync(redis, redisKey);
                            _local.Remove(redisKey);
                        }
                    }

                    var sql = OnDeleteMany(hashGroup.ToList());
                    if (string.IsNullOrEmpty(sql))
                        continue;

                    var refKey = hashGroup.First().GetRedisKey();
                    await _dbExecuteService.Post(world, hashGroup.Key, sql, refKey.ToString());
                }
            });
        }

        public override sealed TModel[] Set(uint world, TModel[] values)
        {
            throw new InvalidOperationException();
        }
    }
}
