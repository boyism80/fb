using System.Collections.Concurrent;
using Dapper;
using Http.Model;
using Http.Redis;
using Http.Service;
using Microsoft.AspNetCore.DataProtection.KeyManagement;
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
        /// <summary>
        /// Lua script for updating value expiry in Redis cache.
        /// Sets a value and conditionally sets expiry if no references exist.
        /// </summary>
        private static readonly string UpdateValueExpiryScript = """
            redis.call('set', @key, @value)

            local contains_refs = redis.call('hexists', @cref, @key)
            if contains_refs == 0 then
                redis.call('expire', @key, @expiry)
            end

            return contains_refs
            """;

        private readonly Dictionary<string, string> _local = new Dictionary<string, string>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _updateValueExpiryScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
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
                    if (!_updateValueExpiryScripts.TryGetValue(redis, out var script))
                    {
                        script = LuaScript.Prepare(UpdateValueExpiryScript).Load(redis.GetServer());
                        _updateValueExpiryScripts[redis] = script;
                    }

                    await redis.Connection.ScriptEvaluateAsync(script, new
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
        /// <summary>
        /// Lua script for updating hash expiry in Redis cache.
        /// Sets multiple hash fields and conditionally sets expiry if no references exist.
        /// </summary>
        private static readonly string UpdateHashExpiryScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]
            local EXPIRY = tonumber(ARGV[1])
            local LENGTH = tonumber(ARGV[2])

            local offset = 2
            for i = 1, LENGTH do
                local field = ARGV[offset + i]
                local value = ARGV[offset + i + 1]
                redis.call('hset', CACHE_KEY, field, value)
                offset = offset + 1
            end

            local contains_refs = redis.call('hexists', COUNT_REFS, CACHE_KEY)
            if contains_refs == 0 then
                redis.call('expire', CACHE_KEY, EXPIRY)
            end

            return {contains_refs}
            """;

        /// <summary>
        /// Lua script for setting a single hash field in Redis cache.
        /// Sets hash field, removes expiry, and increments reference count.
        /// </summary>
        private static readonly string SetHashFieldScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]
            local FIELD = ARGV[1]
            local VALUE = ARGV[2]

            -- Set hash field
            redis.call('hset', CACHE_KEY, FIELD, VALUE)

            -- Remove expiry (persist the key)
            redis.call('persist', CACHE_KEY)

            -- Increment reference count
            redis.call('hincrby', COUNT_REFS, CACHE_KEY, 1)

            return 1
            """;

        /// <summary>
        /// Lua script for setting multiple hash fields in Redis cache.
        /// Sets hash fields, removes expiry, and increments reference count.
        /// </summary>
        private static readonly string SetHashFieldsScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]
            local LENGTH = tonumber(ARGV[1])

            -- Set hash fields
            for i = 1, LENGTH do
                local field = ARGV[i * 2]
                local value = ARGV[i * 2 + 1]
                redis.call('hset', CACHE_KEY, field, value)
            end

            -- Remove expiry (persist the key)
            redis.call('persist', CACHE_KEY)

            -- Increment reference count
            redis.call('hincrby', COUNT_REFS, CACHE_KEY, 1)

            return 1
            """;

        private readonly ConcurrentDictionary<string, ConcurrentDictionary<string, string>> _local = new ConcurrentDictionary<string, ConcurrentDictionary<string, string>>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _updateHashExpiryScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _setHashFieldScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _setHashFieldsScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
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
            return GetLockKey(key.GetRedisKey());
        }

        private static string GetLockKey(RedisKey key)
        {
            return $"lock:{key}";
        }

        /// <summary>
        /// Synchronizes cache from database by loading entities and updating both Redis and local cache.
        /// Retrieves entities from database, stores them in Redis using Lua script, and updates local cache.
        /// </summary>
        /// <param name="redis">The Redis service instance for cache operations.</param>
        /// <param name="key">The key identifying the entities to retrieve and cache.</param>
        /// <returns>A collection of entities loaded from the database.</returns>
        private async Task<IEnumerable<TModel>> SyncCacheFromDatabase(Service.Redis redis, TKey key)
        {
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

                    if (!_updateHashExpiryScripts.TryGetValue(redis, out var script))
                    {
                        script = LuaScript.Prepare(UpdateHashExpiryScript).Load(redis.GetServer());
                        _updateHashExpiryScripts[redis] = script;
                    }

                    var result = await redis.Connection.ScriptEvaluateAsync(script.Hash,
                        keys:
                        [
                            g.Key,
                                new RedisKey(Const.ReferenceCountKey)
                        ],
                        values: values.ToArray());

                    _local.AddOrUpdate(g.Key,
                        new ConcurrentDictionary<string, string>(g.ToDictionary(x => x.GetRedisField().ToString(), x => JsonConvert.SerializeObject(x))),
                        (key, oldValue) => new ConcurrentDictionary<string, string>(g.ToDictionary(x => x.GetRedisField().ToString(), x => JsonConvert.SerializeObject(x))));
                }
            }

            return mysqlValues;
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
                    var newCache = new ConcurrentDictionary<string, string>(redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value)));
                    _local.AddOrUpdate(key.GetRedisKey(), newCache, (k, oldValue) => newCache);
                    if (redisValues.TryGetValue(key.GetRedisField(), out var redisValue))
                    {
                        if (redisValue.Deleted)
                            return null;

                        return redisValue;
                    }
                }

                var mysqlValues = await SyncCacheFromDatabase(redis, key);
                var found = mysqlValues.FirstOrDefault(x =>
                {
                    if (x.GetRedisKey() != key.GetRedisKey())
                        return false;

                    if (x.GetRedisField() != key.GetRedisField())
                        return false;

                    return true;
                });

                if (found == null)
                    return null;

                if (found.Deleted)
                    return null;

                return found;
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
                    var newCache = new ConcurrentDictionary<string, string>(redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value)));
                    _local.AddOrUpdate(key.GetRedisKey(), newCache, (k, oldValue) => newCache);
                    return redisValues.Values.Where(x => !x.Deleted);
                }

                var mysqlValues = await SyncCacheFromDatabase(redis, key);
                return mysqlValues.Where(x => !x.Deleted);
            }
        }

        /// <summary>
        /// Queues a single entity for upsert with Redis hash caching and write-back support.
        /// Updates local cache, Redis hash cache, and schedules database write-back.
        /// If Redis key doesn't exist, calls GetAll to synchronize cache with database.
        /// </summary>
        /// <param name="value">The entity to upsert.</param>
        /// <returns>The same entity instance for method chaining.</returns>
        public override TModel Set(TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                value.UpdatedDate = DateTime.Now;

                var redisKey = value.GetRedisKey();
                var redis = _redisService.Redis(value.GetHash());

                // Use distributed lock to ensure cache synchronization
                await using (await _distributedLock.Lock(GetLockKey(value)))
                {
                    // Check if Redis key exists
                    var keyExists = await redis.Connection.KeyExistsAsync(redisKey);
                    if (!keyExists)
                    {
                        // Redis key doesn't exist, clear local cache and call SyncCacheFromDatabase to synchronize cache with database
                        _local.TryRemove(redisKey, out _);
                        await SyncCacheFromDatabase(redis, value);
                    }

                    // Add the new entity to Redis cache using Lua script
                    if (!_setHashFieldScripts.TryGetValue(redis, out var script))
                    {
                        script = LuaScript.Prepare(SetHashFieldScript).Load(redis.GetServer());
                        _setHashFieldScripts[redis] = script;
                    }

                    await redis.Connection.ScriptEvaluateAsync(script.Hash,
                        keys:
                        [
                            redisKey,
                            new RedisKey(Const.ReferenceCountKey)
                        ],
                        values:
                        [
                            value.GetRedisField(),
                            JsonConvert.SerializeObject(value)
                        ]);

                    // Update local cache (thread-safe with ConcurrentDictionary)
                    var localValues = _local.GetOrAdd(redisKey, _ => new ConcurrentDictionary<string, string>());
                    localValues[value.GetRedisField()] = JsonConvert.SerializeObject(value);
                }

                var sql = OnUpsert(value);
                await _dbExecuteService.Post(value.GetHash(), sql, redisKey.ToString());
            });

            return value;
        }

        /// <summary>
        /// Queues multiple entities for upsert with Redis hash caching and write-back support.
        /// Entities are grouped by hash and Redis key for efficient batch operations with hash structures.
        /// If Redis key doesn't exist, calls GetAll to synchronize cache with database.
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
                    var hash = hashGroup.Key;
                    var redis = _redisService.Redis(hash);

                    // Process keyGroups sequentially
                    foreach (var keyGroup in hashGroup.GroupBy(x => x.GetRedisKey()))
                    {
                        var redisKey = keyGroup.Key;
                        var valueSet = keyGroup.ToDictionary(x => x.GetRedisField(), x => x);

                        // Use distributed lock to ensure cache synchronization
                        await using (await _distributedLock.Lock(GetLockKey(redisKey)))
                        {
                            // Check if Redis key exists
                            var keyExists = await redis.Connection.KeyExistsAsync(redisKey);
                            if (!keyExists)
                            {
                                // Redis key doesn't exist, clear local cache and call SyncCacheFromDatabase to synchronize cache with database
                                _local.TryRemove(redisKey, out _);
                                await SyncCacheFromDatabase(redis, keyGroup.First());
                            }

                            // Add the new entities to Redis cache using Lua script
                            if (!_setHashFieldsScripts.TryGetValue(redis, out var script))
                            {
                                script = LuaScript.Prepare(SetHashFieldsScript).Load(redis.GetServer());
                                _setHashFieldsScripts[redis] = script;
                            }

                            var scriptValues = new List<RedisValue>
                            {
                                valueSet.Count
                            };
                            foreach (var (field, val) in valueSet)
                            {
                                scriptValues.Add(field);
                                scriptValues.Add(JsonConvert.SerializeObject(val));
                            }

                            await redis.Connection.ScriptEvaluateAsync(script.Hash,
                                keys:
                                [
                                    redisKey,
                                    new RedisKey(Const.ReferenceCountKey)
                                ],
                                values: scriptValues.ToArray());

                            // Update local cache (thread-safe with ConcurrentDictionary)
                            var localValues = _local.GetOrAdd(redisKey, _ => new ConcurrentDictionary<string, string>());
                            foreach (var (field, val) in valueSet)
                            {
                                localValues[field] = JsonConvert.SerializeObject(val);
                            }
                        }

                        // Execute OnUpsert and Post for each keyGroup (same hash connection, but grouped by redisKey)
                        var sql = OnUpsert(keyGroup.ToArray());
                        await _dbExecuteService.Post(hash, sql, redisKey.ToString());
                    }
                }
            });

            return values;
        }
    }
}