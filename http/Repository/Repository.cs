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
    /// <summary>
    /// Defines the base contract for all repository implementations.
    /// Provides the fundamental save operation for persisting changes.
    /// </summary>
    public interface IRepository
    {
        /// <summary>
        /// Asynchronously saves all pending changes to the underlying data store.
        /// </summary>
        /// <returns>A task representing the asynchronous save operation.</returns>
        Task SaveChangesAsync();
    }

    /// <summary>
    /// Provides a base implementation for repository pattern with generic model and key types.
    /// Handles basic CRUD operations with database sharding support and buffered writes.
    /// </summary>
    /// <typeparam name="TModel">The model type that implements both IModel and the key interface.</typeparam>
    /// <typeparam name="TKey">The key type that implements IModelKey for sharding support.</typeparam>
    public abstract class Repository<TModel, TKey> : IRepository where TModel : class, IModel, TKey where TKey : IModelKey
    {
        private readonly DbContext _dbContext;

        /// <summary>
        /// Gets the buffer queue for pending database operations.
        /// </summary>
        /// <value>A queue of asynchronous database operations to be executed during save.</value>
        protected readonly Queue<Func<Task>> _buffer = new Queue<Func<Task>>();

        /// <summary>
        /// Initializes a new instance of the <see cref="Repository{TModel, TKey}"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        protected Repository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        /// <summary>
        /// When overridden in a derived class, provides the SQL SELECT statement for retrieving a single entity.
        /// </summary>
        /// <param name="key">The key identifying the entity to retrieve.</param>
        /// <returns>A SQL SELECT statement string.</returns>
        protected abstract string OnSelect(TKey key);

        /// <summary>
        /// When overridden in a derived class, provides the SQL SELECT statement for retrieving multiple entities.
        /// </summary>
        /// <param name="key">The key identifying the entities to retrieve.</param>
        /// <returns>A SQL SELECT statement string.</returns>
        /// <exception cref="NotImplementedException">Thrown when not implemented in derived class.</exception>
        protected virtual string OnSelectBulk(TKey key) { throw new NotImplementedException(); }

        /// <summary>
        /// When overridden in a derived class, provides the SQL UPSERT statement for a single entity.
        /// </summary>
        /// <param name="value">The entity to upsert.</param>
        /// <returns>A SQL UPSERT statement string.</returns>
        protected abstract string OnUpsert(TModel value);

        /// <summary>
        /// When overridden in a derived class, provides the SQL UPSERT statement for multiple entities.
        /// </summary>
        /// <param name="values">The entities to upsert.</param>
        /// <returns>A SQL UPSERT statement string.</returns>
        /// <exception cref="NotImplementedException">Thrown when not implemented in derived class.</exception>
        protected virtual string OnUpsert(TModel[] values) { throw new NotImplementedException(); }

        /// <summary>
        /// Retrieves a single entity from the database using the specified key.
        /// Automatically filters out soft-deleted entities.
        /// </summary>
        /// <param name="key">The key identifying the entity to retrieve.</param>
        /// <returns>The entity if found and not deleted; otherwise, null.</returns>
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

        /// <summary>
        /// Retrieves all entities from the database using the specified key.
        /// Automatically filters out soft-deleted entities.
        /// </summary>
        /// <param name="key">The key identifying the entities to retrieve.</param>
        /// <returns>A collection of entities that are not soft-deleted.</returns>
        protected virtual async Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            await using var conn = _dbContext.Connection(key.GetHash());
            return (await conn.QueryAsync<TModel>(OnSelectBulk(key))).Where(x => !x.Deleted);
        }

        /// <summary>
        /// Queues a single entity for upsert operation during the next save.
        /// The operation is buffered and executed when SaveChangesAsync is called.
        /// </summary>
        /// <param name="value">The entity to upsert.</param>
        /// <returns>The same entity instance for method chaining.</returns>
        public virtual TModel Set(TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                await using var conn = _dbContext.Connection(value.GetHash());
                await conn.ExecuteAsync(OnUpsert(value));
            });
            return value;
        }

        /// <summary>
        /// Queues multiple entities for upsert operation during the next save.
        /// Entities are grouped by shard for efficient batch operations.
        /// </summary>
        /// <param name="values">The entities to upsert.</param>
        /// <returns>The same entity array for method chaining.</returns>
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

        /// <summary>
        /// Asynchronously executes all buffered database operations.
        /// Processes the operation queue until empty.
        /// </summary>
        /// <returns>A task representing the asynchronous save operation.</returns>
        public async Task SaveChangesAsync()
        {
            while (_buffer.TryDequeue(out var func))
            {
                await func();
            }
        }
    }

    /// <summary>
    /// Provides a Redis-backed repository implementation for value-based caching.
    /// Implements a write-through cache pattern with distributed locking and write-back support.
    /// </summary>
    /// <typeparam name="TModel">The model type that implements both IModel and the Redis value key interface.</typeparam>
    /// <typeparam name="TKey">The key type that implements IRedisValueKey for Redis integration.</typeparam>
    public abstract class RedisValueRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisValueKey
    {
        private readonly Dictionary<string, string> _local = new Dictionary<string, string>();
        private readonly RedisService _redisService;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly WriteBackService _dbExecuteService;

        /// <summary>
        /// Initializes a new instance of the <see cref="RedisValueRepository{TModel, TKey}"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        protected RedisValueRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext)
        {
            _redisService = redisService;
            _distributedLock = distributedLock;
            _dbExecuteService = dbExecuteService;
        }

        /// <summary>
        /// Generates a distributed lock key for the specified entity key.
        /// </summary>
        /// <param name="key">The entity key to generate a lock key for.</param>
        /// <returns>A formatted lock key string for distributed locking.</returns>
        private static string GetLockKey(TKey key)
        {
            return $"lock:{key.GetRedisKey()}";
        }

        /// <summary>
        /// Retrieves a single entity with multi-level caching (local, Redis, database).
        /// Uses distributed locking to ensure consistency across cache levels.
        /// </summary>
        /// <param name="key">The key identifying the entity to retrieve.</param>
        /// <returns>The entity if found and not deleted; otherwise, null.</returns>
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

        /// <summary>
        /// This operation is not supported for Redis value repositories.
        /// </summary>
        /// <param name="key">The key parameter (unused).</param>
        /// <returns>Never returns normally.</returns>
        /// <exception cref="InvalidOperationException">Always thrown as this operation is not supported.</exception>
        protected override sealed Task<IEnumerable<TModel>> GetAll(TKey key)
        {
            throw new InvalidOperationException();
        }

        /// <summary>
        /// Queues a single entity for upsert with Redis caching and write-back support.
        /// Updates local cache, Redis cache, and schedules database write-back.
        /// </summary>
        /// <param name="value">The entity to upsert.</param>
        /// <returns>The same entity instance for method chaining.</returns>
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

        /// <summary>
        /// This operation is not supported for Redis value repositories.
        /// </summary>
        /// <param name="values">The values parameter (unused).</param>
        /// <returns>Never returns normally.</returns>
        /// <exception cref="InvalidOperationException">Always thrown as this operation is not supported.</exception>
        public override sealed TModel[] Set(TModel[] values)
        {
            throw new InvalidOperationException();
        }
    }

    /// <summary>
    /// Provides a Redis-backed repository implementation for hash-based caching.
    /// Implements a write-through cache pattern with distributed locking and write-back support for hash structures.
    /// </summary>
    /// <typeparam name="TModel">The model type that implements both IModel and the Redis hash key interface.</typeparam>
    /// <typeparam name="TKey">The key type that implements IRedisHashKey for Redis hash integration.</typeparam>
    public abstract class RedisHashRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisHashKey
    {
        private readonly Dictionary<string, Dictionary<string, string>> _local = new Dictionary<string, Dictionary<string, string>>();
        private readonly RedisService _redisService;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly WriteBackService _dbExecuteService;

        /// <summary>
        /// Initializes a new instance of the <see cref="RedisHashRepository{TModel, TKey}"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        protected RedisHashRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext)
        {
            _redisService = redisService;
            _distributedLock = distributedLock;
            _dbExecuteService = dbExecuteService;
        }

        /// <summary>
        /// Generates a local cache key for the specified entity key.
        /// </summary>
        /// <param name="key">The entity key to generate a local cache key for.</param>
        /// <returns>A formatted local cache key string.</returns>
        private static string GetLocalCacheKey(TKey key)
        {
            return $"{key.GetRedisKey()}:{key.GetRedisField()}";
        }

        /// <summary>
        /// Generates a distributed lock key for the specified entity key.
        /// </summary>
        /// <param name="key">The entity key to generate a lock key for.</param>
        /// <returns>A formatted lock key string for distributed locking.</returns>
        private static string GetLockKey(TKey key)
        {
            return $"lock:{key.GetRedisKey()}:{key.GetRedisField()}";
        }

        /// <summary>
        /// Retrieves a single entity with multi-level caching (local, Redis hash, database).
        /// Uses distributed locking to ensure consistency across cache levels and handles hash-based Redis operations.
        /// </summary>
        /// <param name="key">The key identifying the entity to retrieve.</param>
        /// <returns>The entity if found and not deleted; otherwise, null.</returns>
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

        /// <summary>
        /// Retrieves all entities with multi-level caching (local, Redis hash, database).
        /// Uses distributed locking and handles hash-based Redis operations for bulk retrieval.
        /// </summary>
        /// <param name="key">The key identifying the entities to retrieve.</param>
        /// <returns>A collection of entities that are not soft-deleted.</returns>
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

        /// <summary>
        /// Queues a single entity for upsert with Redis hash caching and write-back support.
        /// Updates local cache, Redis hash cache, and schedules database write-back.
        /// </summary>
        /// <param name="value">The entity to upsert.</param>
        /// <returns>The same entity instance for method chaining.</returns>
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

        /// <summary>
        /// Queues multiple entities for upsert with Redis hash caching and write-back support.
        /// Entities are grouped by hash and Redis key for efficient batch operations with hash structures.
        /// </summary>
        /// <param name="values">The entities to upsert.</param>
        /// <returns>The same entity array for method chaining.</returns>
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
                                cmd.Enqueue(trans => trans.HashIncrementAsync(Const.ReferenceCountKey, redisKey.ToString()));
                            });
                            var sql = OnUpsert(hashGroup.ToArray());
                            await _dbExecuteService.Post(hash, sql, redisKey.ToString());
                        }
                    }
                }
            });

            return values;
        }
    }
}