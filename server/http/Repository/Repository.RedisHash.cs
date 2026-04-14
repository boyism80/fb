using Http.Model;
using Http.Redis;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;
using System.Collections.Concurrent;

namespace Http.Reepository
{
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

        private static readonly string MultiHashGetScript = """
            local n = #KEYS - 1
            local cref = KEYS[n + 1]
            local expiry = tonumber(ARGV[1])
            local out = {}
            for i = 1, n do
                local k = KEYS[i]
                local h = redis.call('HGETALL', k)
                if h and #h > 0 then
                    if redis.call('hexists', cref, k) == 0 then
                        redis.call('EXPIRE', k, expiry)
                    end
                    table.insert(out, tostring(k))
                    table.insert(out, tostring(#h / 2))
                    for j = 1, #h do
                        table.insert(out, h[j])
                    end
                else
                    table.insert(out, tostring(k))
                    table.insert(out, '0')
                end
            end
            return out
            """;

        private readonly ConcurrentDictionary<string, ConcurrentDictionary<string, string>> _local = new ConcurrentDictionary<string, ConcurrentDictionary<string, string>>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _updateHashExpiryScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _setHashFieldScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _setHashFieldsScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _multiHashGetScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
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
            return $"fb:lock:{key}";
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

        protected virtual async Task<IReadOnlyList<TModel>> GetMany(uint world, IReadOnlyList<TKey> keys)
        {
            if (keys == null || keys.Count == 0)
                return Array.Empty<TModel>();

            var result = new List<TModel>();
            var missingKeys = new List<TKey>();

            foreach (var group in keys.Distinct().GroupBy(k => k.GetHash()))
            {
                var hash = group.Key;
                var redis = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                if (redis == null)
                {
                    missingKeys.AddRange(group);
                    continue;
                }

                var keyList = group.ToList();
                var localMiss = new List<TKey>();
                foreach (var key in keyList)
                {
                    var redisKey = key.GetRedisKey();
                    if (_local.TryGetValue(redisKey, out var fields))
                    {
                        var list = fields.Values.Select(x => JsonConvert.DeserializeObject<TModel>(x)).Where(x => x != null && !x.Deleted).ToList();
                        foreach (var m in list)
                        {
                            result.Add(m);
                        }
                    }
                    else
                    {
                        localMiss.Add(key);
                    }
                }

                if (localMiss.Count > 0)
                {
                    var redisKeys = new RedisKey[localMiss.Count + 1];
                    for (var i = 0; i < localMiss.Count; i++)
                    {
                        redisKeys[i] = localMiss[i].GetRedisKey();
                    }
                    redisKeys[localMiss.Count] = Const.ReferenceCountKey;
                    var values = new RedisValue[] { (int)Const.CacheTimeToLive.TotalSeconds };

                    if (!_multiHashGetScripts.TryGetValue(redis, out var multiHashScript))
                    {
                        multiHashScript = LuaScript.Prepare(MultiHashGetScript).Load(redis.GetServer());
                        _multiHashGetScripts[redis] = multiHashScript;
                    }

                    var multiResult = await redis.Connection.ScriptEvaluateAsync(multiHashScript.Hash, redisKeys, values);
                    var flat = (RedisResult[])multiResult;
                    var idx = 0;
                    foreach (var key in localMiss)
                    {
                        if (idx >= flat.Length) break;
                        var keyStr = flat[idx++].ToString();
                        var count = int.Parse(flat[idx++].ToString());
                        if (count == 0)
                        {
                            missingKeys.Add(key);
                            continue;
                        }
                        var hashDict = new Dictionary<RedisValue, string>();
                        for (var c = 0; c < count; c++)
                        {
                            var field = flat[idx++].ToString();
                            var valStr = flat[idx++].ToString();
                            hashDict[(RedisValue)field] = valStr;
                        }
                        var list = new List<TModel>();
                        var localCache = new ConcurrentDictionary<string, string>();
                        foreach (var kv in hashDict)
                        {
                            var model = JsonConvert.DeserializeObject<TModel>(kv.Value);
                            if (model != null && !model.Deleted)
                            {
                                list.Add(model);
                                result.Add(model);
                            }
                            localCache[kv.Key.ToString()] = kv.Value;
                        }
                        _local.AddOrUpdate(key.GetRedisKey().ToString(), localCache, (k, old) => localCache);
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

                    await using (await _distributedLock.Lock(world, GetLockKey(key)))
                    {
                        var hash = key.GetHash();
                        var redisForKey = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                        if (redisForKey != null)
                        {
                            var dataGroup = list.ToDictionary(x => x.GetRedisField(), x => x);
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

                            if (!_updateHashExpiryScripts.TryGetValue(redisForKey, out var script))
                            {
                                script = LuaScript.Prepare(UpdateHashExpiryScript).Load(redisForKey.GetServer());
                                _updateHashExpiryScripts[redisForKey] = script;
                            }

                            await redisForKey.Connection.ScriptEvaluateAsync(script.Hash,
                                keys: [key.GetRedisKey(), new RedisKey(Const.ReferenceCountKey)],
                                values: values.ToArray());

                            _local.AddOrUpdate(key.GetRedisKey(),
                                new ConcurrentDictionary<string, string>(dataGroup.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value))),
                                (k, old) => new ConcurrentDictionary<string, string>(dataGroup.ToDictionary(x => x.Key.ToString(), x => JsonConvert.SerializeObject(x.Value))));
                        }
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
