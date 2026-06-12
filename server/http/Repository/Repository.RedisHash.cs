using Http.Model;
using Http.Reepository.Cache;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository
{
    public abstract class RedisHashRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisHashKey
    {
        private readonly LocalHashMemoryCacheLayer<TModel> _local;
        private readonly RedisHashCacheLayer<TModel> _redis;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly WriteBackService _dbExecuteService;

        protected RedisHashRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext)
        {
            _local = new LocalHashMemoryCacheLayer<TModel>();
            _redis = new RedisHashCacheLayer<TModel>(redisService);
            _distributedLock = distributedLock;
            _dbExecuteService = dbExecuteService;
        }

        private static string GetLockKey(TKey key) => GetLockKey(key.GetRedisKey());

        private static string GetLockKey(RedisKey key) => $"fb:lock:{key}";

        private async Task<IEnumerable<TModel>> SyncCacheFromDatabase(uint world, Service.Redis redis, TKey key)
        {
            var mysqlValues = (await base.GetAll(world, key)).ToList();
            if (mysqlValues.Count == 0)
                return mysqlValues;

            foreach (var g in mysqlValues.GroupBy(x => x.GetRedisKey()))
            {
                await _redis.WriteBackAsync(redis, g.Key, g);
                _local.PutAll(g.Key, g.ToDictionary(x => x.GetRedisField().ToString(), x => JsonConvert.SerializeObject(x)));
            }

            return mysqlValues;
        }

        protected override async Task<TModel> Get(uint world, TKey key)
        {
            await using var _ = await _distributedLock.Lock(world, GetLockKey(key));

            var localValue = _local.TryGetField(key.GetRedisKey(), key.GetRedisField());
            if (localValue != null)
                return localValue;

            var redis = _redis.GetConnection(world, key.GetHash());
            var redisValues = await _redis.TryGetAllAsync(redis, key.GetRedisKey());
            if (redisValues.Count > 0)
            {
                _local.PutAll(key.GetRedisKey(), redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value)));
                if (redisValues.TryGetValue(key.GetRedisField(), out var redisValue))
                {
                    if (redisValue.Deleted)
                        return null;

                    return redisValue;
                }
            }

            var mysqlValues = await SyncCacheFromDatabase(world, redis, key);
            var found = mysqlValues.FirstOrDefault(x =>
                x.GetRedisKey() == key.GetRedisKey() && x.GetRedisField() == key.GetRedisField());

            if (found == null || found.Deleted)
                return null;

            return found;
        }

        protected override async Task<IEnumerable<TModel>> GetAll(uint world, TKey key)
        {
            await using var _ = await _distributedLock.Lock(world, GetLockKey(key));

            if (_local.HasKey(key.GetRedisKey()))
                return _local.GetAll(key.GetRedisKey());

            var redis = _redis.GetConnection(world, key.GetHash());
            var redisValues = await _redis.TryGetFieldsAsync(redis, key.GetRedisKey());
            if (redisValues.Count > 0)
            {
                _local.PutAll(key.GetRedisKey(), redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value)));
                return redisValues.Values.Where(x => !x.Deleted);
            }

            var mysqlValues = await SyncCacheFromDatabase(world, redis, key);
            return mysqlValues.Where(x => !x.Deleted);
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
                    if (_local.HasKey(key.GetRedisKey()))
                    {
                        result.AddRange(_local.GetAll(key.GetRedisKey()));
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

                    foreach (var (redisKey, fields) in redisHits)
                    {
                        _local.PutAll(redisKey, fields);
                        foreach (var json in fields.Values)
                        {
                            var model = JsonConvert.DeserializeObject<TModel>(json);
                            if (model != null && !model.Deleted)
                                result.Add(model);
                        }
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
                    var key = kv.Key;
                    var list = kv.Value;
                    foreach (var entity in list)
                    {
                        result.Add(entity);
                    }

                    if (list.Count == 0)
                        continue;

                    await using var _ = await _distributedLock.Lock(world, GetLockKey(key));

                    var redis = _redis.GetConnection(world, key.GetHash());
                    if (redis != null)
                    {
                        await _redis.WriteBackAsync(redis, key.GetRedisKey(), list);
                        _local.PutAll(key.GetRedisKey(), list.ToDictionary(x => x.GetRedisField().ToString(), x => JsonConvert.SerializeObject(x)));
                    }
                }
            }

            return result;
        }

        public override TModel Set(uint world, TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                value.UpdatedDate = DateTime.Now;

                var redisKey = value.GetRedisKey();
                var redis = _redis.GetConnection(world, value.GetHash());
                if (redis == null)
                    return;

                await using (await _distributedLock.Lock(world, GetLockKey(value)))
                {
                    if (!await _redis.KeyExistsAsync(redis, redisKey))
                    {
                        _local.Remove(redisKey);
                        await SyncCacheFromDatabase(world, redis, value);
                    }

                    await _redis.SetFieldAsync(redis, redisKey, value.GetRedisField(), value);
                    _local.PutField(redisKey, value.GetRedisField(), value);
                }

                var sql = OnUpsert(value);
                await _dbExecuteService.Post(world, value.GetHash(), sql, redisKey.ToString());
            });

            return value;
        }

        public override TModel[] Set(uint world, TModel[] values)
        {
            _buffer.Enqueue(async () =>
            {
                foreach (var value in values)
                {
                    value.UpdatedDate = DateTime.Now;
                }

                foreach (var hashGroup in values.GroupBy(x => x.GetHash()))
                {
                    var redis = _redis.GetConnection(world, hashGroup.Key);
                    if (redis == null)
                        continue;

                    foreach (var keyGroup in hashGroup.GroupBy(x => x.GetRedisKey()))
                    {
                        var redisKey = keyGroup.Key;
                        var valueSet = keyGroup.ToDictionary(x => x.GetRedisField(), x => x);

                        await using (await _distributedLock.Lock(world, GetLockKey(redisKey)))
                        {
                            if (!await _redis.KeyExistsAsync(redis, redisKey))
                            {
                                _local.Remove(redisKey);
                                await SyncCacheFromDatabase(world, redis, keyGroup.First());
                            }

                            await _redis.SetFieldsAsync(redis, redisKey, valueSet);
                            foreach (var (field, val) in valueSet)
                            {
                                _local.PutField(redisKey, field, val);
                            }
                        }

                        var sql = OnUpsert(keyGroup.ToArray());
                        await _dbExecuteService.Post(world, hashGroup.Key, sql, redisKey.ToString());
                    }
                }
            });

            return values;
        }
    }
}
