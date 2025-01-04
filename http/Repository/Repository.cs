using Dapper;
using Http.Model;
using Http.Redis;
using Http.Service;
using Medallion.Threading;
using Medallion.Threading.Redis;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository
{
    public interface IRepository
    {
        Task SaveChangesAsync();
    }

    public abstract class Repository<TModel, TKey> : IRepository where TModel : class, IModel, TKey where TKey : IModelKey
    {
        private readonly DbContext _dbContext;
        protected readonly Queue<Func<Task>> _buffer = new Queue<Func<Task>>();

        protected Repository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        protected abstract string OnSelect(TKey key);
        protected virtual string OnSelectBulk(TKey key) { throw new NotImplementedException(); }
        protected abstract string OnUpsert(TModel value);
        protected virtual string OnUpsert(TModel[] values) { throw new NotImplementedException(); }

        protected virtual async Task<TModel> Get(TKey key)
        {
            await using var conn = _dbContext.Connection(key.GetHash());
            var value = await conn.QuerySingleOrDefaultAsync<TModel>(OnSelect(key));
            if (value == null)
                return null;

            if (value.Deleted)
                return null;

            return value;
        }

        protected virtual async Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            await using var conn = _dbContext.Connection(key.GetHash());
            return (await conn.QueryAsync<TModel>(OnSelectBulk(key))).Where(x => !x.Deleted);
        }

        public virtual TModel Set(TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                await using var conn = _dbContext.Connection(value.GetHash());
                await conn.ExecuteAsync(OnUpsert(value));
            });
            return value;
        }

        public virtual TModel[] Set(TModel[] values)
        {
            _buffer.Enqueue(async () =>
            {
                foreach (var (conn, items) in _dbContext.Connections(values, value => value.GetHash()))
                {
                    await conn.ExecuteAsync(OnUpsert(items));
                }
            });

            return values;
        }

        public async Task SaveChangesAsync()
        {
            while (_buffer.TryDequeue(out var func))
            {
                await func();
            }
        }
    }

    public abstract class RedisValueRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisValueKey
    {
        private readonly Dictionary<string, string> _local = new Dictionary<string, string>();
        private readonly RedisService _redisService;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly WriteBackService _dbExecuteService;

        protected RedisValueRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext)
        {
            _redisService = redisService;
            _distributedLock = distributedLock;
            _dbExecuteService = dbExecuteService;
        }

        private static string GetLockKey(TKey key)
        {
            return $"lock:{key.GetRedisKey()}";
        }

        protected override async Task<TModel> Get(TKey key)
        {
            await using (await _distributedLock.Lock(GetLockKey(key)))
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValue))
                {
                    var value = JsonConvert.DeserializeObject<TModel>(localValue);
                    if (value.Deleted)
                        return null;

                    return value;
                }

                var redis = _redisService.Redis(key);
                var redisValues = await redis.Connection.JsonGetAsync<TModel>(key.GetRedisKey());
                if (redisValues != null)
                {
                    if (redisValues.Deleted)
                        return null;

                    return redisValues;
                }

                var mysqlValue = await base.Get(key);
                if (mysqlValue != null)
                {
                    await _redisService.Redis(key).ScriptEvaluateAsync("update_value_expiry.lua", new
                    {
                        key = key.GetRedisKey(),
                        value = JsonConvert.SerializeObject(mysqlValue),
                        cref = new RedisKey(Const.ReferenceCountKey),
                        expiry = (int)Const.CacheTimeToLive.TotalSeconds
                    });

                    if (mysqlValue.Deleted)
                        return null;

                    return mysqlValue;
                }

                return null;
            }
        }

        protected override sealed Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            throw new InvalidOperationException();
        }

        public override TModel Set(TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                value.UpdatedDate = DateTime.Now;

                var redis = _redisService.Redis(value).Connection;
                await redis.TransactAsync(cmd =>
                {
                    cmd.Enqueue(trans => trans.JsonSetAsync(value.GetRedisKey(), value));
                    cmd.Enqueue(trans => trans.KeyExpireAsync(value.GetRedisKey(), expiry: (TimeSpan?)null));
                    cmd.Enqueue(trans => trans.HashIncrementAsync(Const.ReferenceCountKey, value.GetRedisKey().ToString()));
                });

                _local[value.GetRedisKey()] = JsonConvert.SerializeObject(value);

                var sql = OnUpsert(value);
                await _dbExecuteService.Post(value.GetHash(), sql, value.GetRedisKey().ToString());
            });
            return value;
        }

        public override sealed TModel[] Set(TModel[] values)
        {
            throw new InvalidOperationException();
        }
    }

    public abstract class RedisHashRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisHashKey
    {
        private readonly Dictionary<string, Dictionary<string, string>> _local = new Dictionary<string, Dictionary<string, string>>();
        private readonly RedisService _redisService;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly WriteBackService _dbExecuteService;

        protected RedisHashRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext)
        {
            _redisService = redisService;
            _distributedLock = distributedLock;
            _dbExecuteService = dbExecuteService;
        }

        private static string GetLocalCacheKey(TKey key)
        {
            return $"{key.GetRedisKey()}:{key.GetRedisField()}";
        }

        private static string GetLockKey(TKey key)
        {
            return $"lock:{key.GetRedisKey()}:{key.GetRedisField()}";
        }

        protected override async Task<TModel> Get(TKey key)
        {
            await using (await _distributedLock.Lock(GetLockKey(key)))
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValues) && localValues.TryGetValue(key.GetRedisField(), out var localValue))
                {
                    var value = JsonConvert.DeserializeObject<TModel>(localValue);
                    if (value.Deleted)
                        return null;

                    return value;
                }

                var redis = _redisService.Redis(key.GetHash());
                var redisValues = await redis.Connection.JsonHashGetAllAsync<TModel>(key.GetRedisKey());
                if (redisValues.Count > 0)
                {
                    _local[key.GetRedisKey()] = redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value));
                    if (redisValues.TryGetValue(key.GetRedisField(), out var redisValue))
                    {
                        if (redisValue.Deleted)
                            return null;

                        return redisValue;
                    }
                }

                var mysqlValues = await base.GetAll(key);
                if (mysqlValues.Any())
                {
                    foreach (var g in mysqlValues.GroupBy(x => x.GetRedisKey()))
                    {
                        var dataGroup = g.ToDictionary(x => x.GetRedisField(), x => x);
                        var values = new List<RedisValue>
                        {
                            (int)Const.CacheTimeToLive.TotalSeconds,
                            dataGroup.Count
                        };
                        foreach (var (k, v) in dataGroup)
                        {
                            values.Add(k);
                            values.Add(JsonConvert.SerializeObject(v));
                        }
                        var result = await redis.ScriptEvaluateAsync("update_hash_expiry.lua",
                            keys:
                            [
                                g.Key,
                                new RedisKey(Const.ReferenceCountKey)
                            ],
                            values: values.ToArray());
                        _local[g.Key] = g.ToDictionary(x => x.GetRedisField().ToString(), x => JsonConvert.SerializeObject(x));
                    }

                    var value = mysqlValues.FirstOrDefault(x =>
                    {
                        if (x.GetRedisKey() != key.GetRedisKey())
                            return false;

                        if (x.GetRedisField() != key.GetRedisField())
                            return false;

                        return true;
                    });

                    if (value == null)
                        return null;

                    if (value.Deleted)
                        return null;

                    return value;
                }

                return null;
            }
        }

        protected override async Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            await using (await _distributedLock.Lock(GetLockKey(key)))
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValues))
                    return localValues.Values.Select(x => JsonConvert.DeserializeObject<TModel>(x)).Where(x => !x.Deleted);

                var redis = _redisService.Redis(key.GetHash());
                var redisValues = await redis.Connection.JsonHashGetAsync<TModel>(key.GetRedisKey());
                if (redisValues.Count > 0)
                {
                    _local[key.GetRedisKey()] = redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value));
                    return redisValues.Values.Where(x => !x.Deleted);
                }

                var mysqlValues = await base.GetAll(key);
                if (mysqlValues.Any())
                {
                    foreach (var g in mysqlValues.GroupBy(x => x.GetRedisKey()))
                    {
                        var dataGroup = g.ToDictionary(x => new RedisValue(x.GetRedisField()), x => x);
                        var values = new List<RedisValue>
                        {
                            (int)Const.CacheTimeToLive.TotalSeconds,
                            dataGroup.Count
                        };
                        foreach (var (k, v) in dataGroup)
                        {
                            values.Add(k);
                            values.Add(JsonConvert.SerializeObject(v));
                        }
                        var result = await redis.ScriptEvaluateAsync("update_hash_expiry.lua",
                            keys:
                            [
                                g.Key,
                                new RedisKey(Const.ReferenceCountKey)
                            ],
                            values: values.ToArray());

                        _local[g.Key] = g.ToDictionary(x => x.GetRedisField().ToString(), x => JsonConvert.SerializeObject(x));
                    }

                    return mysqlValues.Where(x => !x.Deleted);
                }

                return [];
            }
        }

        public override TModel Set(TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                value.UpdatedDate = DateTime.Now;

                var redis = _redisService.Redis(value.GetHash()).Connection;
                await redis.TransactAsync(cmd =>
                {
                    cmd.Enqueue(trans => trans.JsonHashSetAsync(value.GetRedisKey(), value.GetRedisField(), value));
                    cmd.Enqueue(trans => trans.KeyExpireAsync(value.GetRedisKey(), expiry: (TimeSpan?)null));
                    cmd.Enqueue(trans => trans.HashIncrementAsync(Const.ReferenceCountKey, value.GetRedisKey().ToString()));
                });

                if (_local.TryGetValue(value.GetRedisKey(), out var localValues))
                    localValues[value.GetRedisField()] = JsonConvert.SerializeObject(value);

                var sql = OnUpsert(value);
                await _dbExecuteService.Post(value.GetHash(), sql, value.GetRedisKey().ToString());
            });

            return value;
        }

        public override TModel[] Set(TModel[] values)
        {
            _buffer.Enqueue(async () =>
            {
                foreach (var value in values)
                {
                    value.UpdatedDate = DateTime.Now;
                }

                foreach (var hashGroup in values.GroupBy(x => x.GetHash()))
                {
                    if (hashGroup.GroupBy(x => x.GetHash()).Count() > 1)
                        throw new InvalidOperationException();

                    var hash = hashGroup.Key;
                    var redis = _redisService.Redis(hash).Connection;

                    foreach (var modGroup in hashGroup.GroupBy(x => (int)(x.GetHash() % _redisService.ShardSize)))
                    {
                        foreach (var keyGroup in modGroup.GroupBy(x => x.GetRedisKey()))
                        {
                            var redisKey = keyGroup.Key;
                            var valueSet = keyGroup.ToDictionary(x => x.GetRedisField(), x => x);
                            await redis.TransactAsync(cmd =>
                            {
                                cmd.Enqueue(trans => trans.JsonHashSetAsync(redisKey, valueSet));
                                cmd.Enqueue(trans => trans.KeyExpireAsync(redisKey, expiry: (TimeSpan?)null));
                                cmd.Enqueue(trans => trans.HashIncrementAsync(Const.ReferenceCountKey, hash.ToString()));
                            });
                            var sql = OnUpsert(hashGroup.ToArray());
                            await _dbExecuteService.Post(hash, sql, hash.ToString());
                        }
                    }
                }
            });

            return values;
        }
    }
}