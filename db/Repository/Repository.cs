using Dapper;
using Db.Model;
using Db.Service;
using http.Redis;
using http.Service;
using StackExchange.Redis;

namespace Db.Reepository
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

    public abstract class RedisValueRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisCacheKey
    {
        private readonly RedisService _redisService;
        private readonly DbExecuteService _dbExecuteService;

        protected RedisValueRepository(DbContext dbContext,
            RedisService redisService,
            DbExecuteService dbExecuteService) : base(dbContext)
        {
            _redisService = redisService;
            _dbExecuteService = dbExecuteService;
        }

        protected override async Task<TModel> Get(TKey key)
        {
            var connRedis = _redisService.Connection;
            var redisValues = await connRedis.JsonGetAsync<TModel>(key.GetRedisKey());
            if (redisValues != null)
                return redisValues;

            var mysqlValue = await base.Get(key);
            if (mysqlValue != null)
            {
                await connRedis.JsonSetAsync(key.GetRedisKey(), mysqlValue);
                return await Get(key);
            }

            return null;
        }

        protected override sealed Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            throw new InvalidOperationException();
        }

        public override async Task Set(TModel value)
        {
            var connRedis = _redisService.Connection;
            await connRedis.TransactAsync(cmd =>
            {
                cmd.Enqueue(trans => trans.JsonSetAsync(value.GetRedisKey(), value));
                cmd.Enqueue(trans => trans.KeyExpireAsync(value.GetRedisKey(), expiry: (TimeSpan?)null));
                cmd.Enqueue(trans => trans.HashIncrementAsync(Db.Redis.Const.ReferenceCountKey, value.GetRedisKey().ToString()));
            });

            var sql = OnUpsert(value);
            await _dbExecuteService.Post(value.GetDbKey(), sql, value.GetRedisKey().ToString());
        }

        public override sealed Task Set(TModel[] values)
        {
            throw new InvalidOperationException();
        }
    }

    public abstract class RedisHashRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisCacheKey
    {
        private readonly RedisService _redisService;
        private readonly DbExecuteService _dbExecuteService;

        protected RedisHashRepository(DbContext dbContext,
            RedisService redisService,
            DbExecuteService dbExecuteService) : base(dbContext)
        {
            _redisService = redisService;
            _dbExecuteService = dbExecuteService;
        }

        protected override async Task<TModel> Get(TKey key)
        {
            var connRedis = _redisService.Connection;
            var redisValues = await connRedis.JsonHashGetAsync<TModel>(key.GetRedisKey(), key.GetRedisField());
            if (redisValues != null)
                return redisValues;

            var mysqlValues = await base.GetAll(key);
            if (mysqlValues.Any())
            {
                foreach (var g in mysqlValues.GroupBy(x => x.GetRedisKey()))
                {
                    await connRedis.JsonHashSetAsync(g.Key, g.ToDictionary(x => x.GetRedisField(), x => x));
                }

                return await Get(key);
            }

            return null;
        }

        protected override async Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            var connRedis = _redisService.Connection;
            var redisValues = await connRedis.JsonHashGetAsync<TModel>(key.GetRedisKey());
            if (redisValues.Count > 0)
                return redisValues.Values;

            var mysqlValues = await base.GetAll(key);
            if (mysqlValues.Any())
            {
                foreach (var g in mysqlValues.GroupBy(x => x.GetRedisKey()))
                {
                    await connRedis.JsonHashSetAsync(g.Key, g.ToDictionary(x => new RedisValue(x.GetRedisField()), x => x));
                }

                return await GetAll(key);
            }

            return [];
        }

        public override async Task Set(TModel value)
        {
            var connRedis = _redisService.Connection;
            await connRedis.TransactAsync(cmd =>
            {
                cmd.Enqueue(trans => trans.JsonHashSetAsync(value.GetRedisKey(), value.GetRedisField(), value));
                cmd.Enqueue(trans => trans.KeyExpireAsync(value.GetRedisKey(), expiry: (TimeSpan?)null));
                cmd.Enqueue(trans => trans.HashIncrementAsync(Db.Redis.Const.ReferenceCountKey, value.GetRedisKey().ToString()));
            });
            var sql = OnUpsert(value);
            await _dbExecuteService.Post(value.GetDbKey(), sql, value.GetRedisKey().ToString());
        }

        public override async Task Set(TModel[] values)
        {
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
                    cmd.Enqueue(trans => trans.HashIncrementAsync(Db.Redis.Const.ReferenceCountKey, redisKey.ToString()));
                });
                var dbKey = g.Select(x => x.GetDbKey()).First();
                var sql = OnUpsert(g.ToArray());
                await _dbExecuteService.Post(dbKey, sql, redisKey.ToString());
            }
        }
    }
}