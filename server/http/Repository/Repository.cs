using Dapper;
using Http.Model;
using Http.Redis;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;
using System.Collections.Concurrent;

namespace Http.Reepository
{
    public interface IRepository
    {
        Task SaveChangesAsync();
    }

    public abstract class Repository<TModel, TKey> : IRepository where TModel : class, IModel, TKey where TKey : IModelKey
    {
        protected readonly DbContext _dbContext;

        protected readonly Queue<Func<Task>> _buffer = new Queue<Func<Task>>();

        protected Repository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        protected abstract string OnSelect(TKey key);

        protected virtual string OnSelectBulk(TKey key) { throw new NotImplementedException(); }

        protected abstract string OnUpsert(TModel value);

        protected virtual string OnUpsert(TModel[] values) { throw new NotImplementedException(); }

        protected virtual async Task<TModel> Get(uint world, TKey key)
        {
            var hash = key.GetHash();
            await using var conn = hash == null ? _dbContext.GetGlobalConnection(world) : _dbContext.GetShardConnection(world, hash.Value);
            var value = await conn.QuerySingleOrDefaultAsync<TModel>(OnSelect(key));
            if (value == null)
                return null;

            if (value.Deleted)
                return null;

            return value;
        }

        protected virtual async Task<IEnumerable<TModel>> GetAll(uint world, TKey key)
        {
            var hash = key.GetHash();
            await using var conn = hash == null ? _dbContext.GetGlobalConnection(world) : _dbContext.GetShardConnection(world, hash.Value);
            return (await conn.QueryAsync<TModel>(OnSelectBulk(key))).Where(x => !x.Deleted);
        }

        public virtual TModel Set(uint world, TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                var hash = value.GetHash();
                await using var conn = hash == null ? _dbContext.GetGlobalConnection(world) : _dbContext.GetShardConnection(world, hash.Value);
                await conn.ExecuteAsync(OnUpsert(value));
            });
            return value;
        }

        public virtual TModel[] Set(uint world, TModel[] values)
        {
            _buffer.Enqueue(async () =>
            {
                foreach (var (conn, items) in _dbContext.GetShardConnections(world, values, value => value.GetHash()))
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

        protected override async Task<TModel> Get(uint world, TKey key)
        {
            await using (await _distributedLock.Lock(world, GetLockKey(key)))
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValue))
                {
                    var value = JsonConvert.DeserializeObject<TModel>(localValue);
                    if (value.Deleted)
                        return null;

                    return value;
                }

                var hash = key.GetHash();
                var redis = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                if (redis == null)
                    return null;

                var redisValues = await redis.Connection.JsonGetAsync<TModel>(key.GetRedisKey());
                if (redisValues != null)
                {
                    if (redisValues.Deleted)
                        return null;

                    return redisValues;
                }

                var mysqlValue = await base.Get(world, key);
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

        protected override sealed Task<IEnumerable<TModel>> GetAll(uint world, TKey key)
        {
            throw new InvalidOperationException();
        }

        public override TModel Set(uint world, TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                value.UpdatedDate = DateTime.Now;

                var hash = value.GetHash();
                var redis = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                if (redis == null)
                    return;

                await redis.Connection.TransactAsync(cmd =>
                {
                    cmd.Enqueue(trans => trans.JsonSetAsync(value.GetRedisKey(), value));
                    cmd.Enqueue(trans => trans.KeyExpireAsync(value.GetRedisKey(), expiry: (TimeSpan?)null));
                    cmd.Enqueue(trans => trans.HashIncrementAsync(Const.ReferenceCountKey, value.GetRedisKey().ToString()));
                });

                _local[value.GetRedisKey()] = JsonConvert.SerializeObject(value);

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

    public abstract class RedisHashRepository<TModel, TKey> : Repository<TModel, TKey> where TModel : class, IModel, TKey where TKey : IRedisHashKey
    {
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
            return GetLockKey(key.GetRedisKey());
        }

        private static string GetLockKey(RedisKey key)
        {
            return $"lock:{key}";
        }

        private async Task<IEnumerable<TModel>> SyncCacheFromDatabase(uint world, Service.Redis redis, TKey key)
        {
            var mysqlValues = await base.GetAll(world, key);
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

        protected override async Task<TModel> Get(uint world, TKey key)
        {
            await using (await _distributedLock.Lock(world, GetLockKey(key)))
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValues) && localValues.TryGetValue(key.GetRedisField(), out var localValue))
                {
                    var value = JsonConvert.DeserializeObject<TModel>(localValue);
                    if (value.Deleted)
                        return null;

                    return value;
                }

                var hash = key.GetHash();
                var redis = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                if (redis == null)
                    return null;

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

                var mysqlValues = await SyncCacheFromDatabase(world, redis, key);
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

        protected override async Task<IEnumerable<TModel>> GetAll(uint world, TKey key)
        {
            await using (await _distributedLock.Lock(world, GetLockKey(key)))
            {
                if (_local.TryGetValue(key.GetRedisKey(), out var localValues))
                    return localValues.Values.Select(x => JsonConvert.DeserializeObject<TModel>(x)).Where(x => !x.Deleted);

                var hash = key.GetHash();
                var redis = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                if (redis == null)
                    return Enumerable.Empty<TModel>();

                var redisValues = await redis.Connection.JsonHashGetAsync<TModel>(key.GetRedisKey());
                if (redisValues.Count > 0)
                {
                    var newCache = new ConcurrentDictionary<string, string>(redisValues.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value)));
                    _local.AddOrUpdate(key.GetRedisKey(), newCache, (k, oldValue) => newCache);
                    return redisValues.Values.Where(x => !x.Deleted);
                }

                var mysqlValues = await SyncCacheFromDatabase(world, redis, key);
                return mysqlValues.Where(x => !x.Deleted);
            }
        }

        public override TModel Set(uint world, TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                value.UpdatedDate = DateTime.Now;

                var redisKey = value.GetRedisKey();
                var hash = value.GetHash();
                var redis = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                if (redis == null)
                    return;

                // Use distributed lock to ensure cache synchronization
                await using (await _distributedLock.Lock(world, GetLockKey(value)))
                {
                    // Check if Redis key exists
                    var keyExists = await redis.Connection.KeyExistsAsync(redisKey);
                    if (!keyExists)
                    {
                        // Redis key doesn't exist, clear local cache and call SyncCacheFromDatabase to synchronize cache with database
                        _local.TryRemove(redisKey, out _);
                        await SyncCacheFromDatabase(world, redis, value);
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
                    var hash = hashGroup.Key;
                    var redis = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                    if (redis == null)
                        continue;

                    // Process keyGroups sequentially
                    foreach (var keyGroup in hashGroup.GroupBy(x => x.GetRedisKey()))
                    {
                        var redisKey = keyGroup.Key;
                        var valueSet = keyGroup.ToDictionary(x => x.GetRedisField(), x => x);

                        // Use distributed lock to ensure cache synchronization
                        await using (await _distributedLock.Lock(world, GetLockKey(redisKey)))
                        {
                            // Check if Redis key exists
                            var keyExists = await redis.Connection.KeyExistsAsync(redisKey);
                            if (!keyExists)
                            {
                                // Redis key doesn't exist, clear local cache and call SyncCacheFromDatabase to synchronize cache with database
                                _local.TryRemove(redisKey, out _);
                                await SyncCacheFromDatabase(world, redis, keyGroup.First());
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
                        await _dbExecuteService.Post(world, hash, sql, redisKey.ToString());
                    }
                }
            });

            return values;
        }
    }
}