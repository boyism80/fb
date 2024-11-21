using Dapper;
using Http.Model;
using Http.Service;
using Http.Redis;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository
{
    public interface IRepository
    { }

    public abstract class Repository<TModel, TKey> : IRepository where TModel : IModel, TKey where TKey : IModelKey
    {
        private readonly DbContext _dbContext;

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
            await using var conn = _dbContext.Connection(key.GetDbKey());
            return await conn.QuerySingleOrDefaultAsync<TModel>(OnSelect(key));
        }

        protected virtual async Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            await using var conn = _dbContext.Connection(key.GetDbKey());
            return await conn.QueryAsync<TModel>(OnSelectBulk(key));
        }

        public virtual async Task Set(TModel value)
        {
            await using var conn = _dbContext.Connection(value.GetDbKey());
            await conn.ExecuteAsync(OnUpsert(value));
        }

        public virtual async Task Set(TModel[] values)
        {
            foreach (var (conn, items) in _dbContext.Connections(values, value => value.GetDbKey()))
            {
                await conn.ExecuteAsync(OnUpsert(items));
            }
        }
    }

    public abstract class RedisValueRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisValueKey
    {
        private readonly Dictionary<string, string> _local = new Dictionary<string, string>();
        private readonly RedisService _redisService;
        private readonly DbExecuteService _dbExecuteService;

        protected RedisValueRepository(DbContext dbContext,
            RedisService redisService,
            DbExecuteService dbExecuteService) : base(dbContext)
        {
            _redisService = redisService;
            _dbExecuteService = dbExecuteService;
        }

        private static string GetLockKey(TKey key)
        {
            return $"lock:{key.GetRedisKey()}";
        }

        protected override async Task<TModel> Get(TKey key)
        {
            var connRedis = _redisService.Connection;
            return await connRedis.Sync(GetLockKey(key), async () =>
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValue))
                    return JsonConvert.DeserializeObject<TModel>(localValue);

                var redisValues = await connRedis.JsonGetAsync<TModel>(key.GetRedisKey());
                if (redisValues != null)
                    return redisValues;

                var mysqlValue = await base.Get(key);
                if (mysqlValue != null)
                {
                    await connRedis.JsonSetAsync(key.GetRedisKey(), mysqlValue);
                    return mysqlValue;
                }

                return null;
            });
        }

        protected override sealed Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            throw new InvalidOperationException();
        }

        public override async Task Set(TModel value)
        {
            value.UpdatedDate = DateTime.Now;

            var connRedis = _redisService.Connection;
            await connRedis.TransactAsync(cmd =>
            {
                cmd.Enqueue(trans => trans.JsonSetAsync(value.GetRedisKey(), value));
                cmd.Enqueue(trans => trans.KeyExpireAsync(value.GetRedisKey(), expiry: (TimeSpan?)null));
                cmd.Enqueue(trans => trans.HashIncrementAsync(Http.Redis.Const.ReferenceCountKey, value.GetRedisKey().ToString()));
            });

            _local[value.GetRedisKey()] = JsonConvert.SerializeObject(value);

            var sql = OnUpsert(value);
            await _dbExecuteService.Post(value.GetDbKey(), sql, value.GetRedisKey().ToString());
        }

        public override sealed Task Set(TModel[] values)
        {
            throw new InvalidOperationException();
        }
    }

    public abstract class RedisHashRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisHashKey
    {
        private readonly Dictionary<string, Dictionary<string, string>> _local = new Dictionary<string, Dictionary<string, string>>();
        private readonly RedisService _redisService;
        private readonly DbExecuteService _dbExecuteService;

        protected RedisHashRepository(DbContext dbContext,
            RedisService redisService,
            DbExecuteService dbExecuteService) : base(dbContext)
        {
            _redisService = redisService;
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
            var connRedis = _redisService.Connection;
            return await connRedis.Sync(GetLockKey(key), async () =>
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValues) && localValues.TryGetValue(key.GetRedisField(), out var localValue))
                    return JsonConvert.DeserializeObject<TModel>(localValue);

                var redisValues = await connRedis.JsonHashGetAllAsync<TModel>(key.GetRedisKey());
                if (redisValues.Count > 0)
                {
                    _local[key.GetRedisKey()] = redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value));
                    if (redisValues.TryGetValue(key.GetRedisField(), out var redisValue))
                        return redisValue;
                }

                var mysqlValues = await base.GetAll(key);
                if (mysqlValues.Any())
                {
                    foreach (var g in mysqlValues.GroupBy(x => x.GetRedisKey()))
                    {
                        await connRedis.JsonHashSetAsync(g.Key, g.ToDictionary(x => x.GetRedisField(), x => x));
                        _local[g.Key] = g.ToDictionary(x => x.GetRedisField().ToString(), x => JsonConvert.SerializeObject(x));
                    }

                    return mysqlValues.FirstOrDefault(x =>
                    {
                        if (x.GetRedisKey() != key.GetRedisKey())
                            return false;

                        if (x.GetRedisField() != key.GetRedisField())
                            return false;

                        return true;
                    });
                }

                return null;
            });
        }

        protected override async Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            var connRedis = _redisService.Connection;
            return await connRedis.Sync(GetLockKey(key), async () =>
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValues))
                    return localValues.Values.Select(x => JsonConvert.DeserializeObject<TModel>(x));

                var redisValues = await connRedis.JsonHashGetAsync<TModel>(key.GetRedisKey());
                if (redisValues.Count > 0)
                {
                    _local[key.GetRedisKey()] = redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value));
                    return redisValues.Values;
                }

                var mysqlValues = await base.GetAll(key);
                if (mysqlValues.Any())
                {
                    foreach (var g in mysqlValues.GroupBy(x => x.GetRedisKey()))
                    {
                        await connRedis.JsonHashSetAsync(g.Key, g.ToDictionary(x => new RedisValue(x.GetRedisField()), x => x));
                        _local[g.Key] = g.ToDictionary(x => x.GetRedisField().ToString(), x => JsonConvert.SerializeObject(x));
                    }

                    return mysqlValues;
                }

                return [];
            });
        }

        public override async Task Set(TModel value)
        {
            value.UpdatedDate = DateTime.Now;

            var connRedis = _redisService.Connection;
            await connRedis.TransactAsync(cmd =>
            {
                cmd.Enqueue(trans => trans.JsonHashSetAsync(value.GetRedisKey(), value.GetRedisField(), value));
                cmd.Enqueue(trans => trans.KeyExpireAsync(value.GetRedisKey(), expiry: (TimeSpan?)null));
                cmd.Enqueue(trans => trans.HashIncrementAsync(Http.Redis.Const.ReferenceCountKey, value.GetRedisKey().ToString()));
            });

            if (_local.TryGetValue(value.GetRedisKey(), out var localValues))
                localValues.Add(value.GetRedisField(), JsonConvert.SerializeObject(value));

            var sql = OnUpsert(value);
            await _dbExecuteService.Post(value.GetDbKey(), sql, value.GetRedisKey().ToString());
        }

        public override async Task Set(TModel[] values)
        {
            foreach (var value in values)
            {
                value.UpdatedDate = DateTime.Now;
            }

            var connRedis = _redisService.Connection;
            foreach (var g in values.GroupBy(x => x.GetRedisKey()))
            {
                if (g.GroupBy(x => x.GetDbKey()).Count() > 1)
                    throw new InvalidOperationException();

                var redisKey = g.Key;
                var valueSet = g.ToDictionary(x => x.GetRedisField(), x => x);
                await connRedis.TransactAsync(cmd =>
                {
                    cmd.Enqueue(trans => trans.JsonHashSetAsync(redisKey, valueSet));
                    cmd.Enqueue(trans => trans.KeyExpireAsync(redisKey, expiry: (TimeSpan?)null));
                    cmd.Enqueue(trans => trans.HashIncrementAsync(Http.Redis.Const.ReferenceCountKey, redisKey.ToString()));
                });
                var dbKey = g.Select(x => x.GetDbKey()).First();
                var sql = OnUpsert(g.ToArray());
                await _dbExecuteService.Post(dbKey, sql, redisKey.ToString());
            }
        }
    }
}