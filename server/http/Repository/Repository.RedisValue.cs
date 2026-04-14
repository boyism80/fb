using Http.Model;
using Http.Redis;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository
{
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

        private static readonly string MultiValueGetScript = """
            local n = #KEYS - 1
            local cref = KEYS[n + 1]
            local expiry = tonumber(ARGV[1])
            local results = {}
            for i = 1, n do
                local k = KEYS[i]
                local v = redis.call('GET', k)
                if v and v ~= '' then
                    if redis.call('hexists', cref, k) == 0 then
                        redis.call('EXPIRE', k, expiry)
                    end
                    table.insert(results, v)
                else
                    table.insert(results, false)
                end
            end
            return results
            """;

        private readonly Dictionary<string, string> _local = new Dictionary<string, string>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _updateValueExpiryScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _multiValueGetScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
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
            return $"fb:lock:{key.GetRedisKey()}";
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
                    if (_local.TryGetValue(redisKey, out var json))
                    {
                        var v = JsonConvert.DeserializeObject<TModel>(json);
                        if (v != null && !v.Deleted)
                            result.Add(v);
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

                    if (!_multiValueGetScripts.TryGetValue(redis, out var multiGetScript))
                    {
                        multiGetScript = LuaScript.Prepare(MultiValueGetScript).Load(redis.GetServer());
                        _multiValueGetScripts[redis] = multiGetScript;
                    }

                    var multiResult = await redis.Connection.ScriptEvaluateAsync(multiGetScript.Hash, redisKeys, values);
                    var elements = (RedisResult[])multiResult;
                    for (var i = 0; i < localMiss.Count; i++)
                    {
                        var key = localMiss[i];
                        var elem = elements[i];
                        if (elem.IsNull)
                        {
                            missingKeys.Add(key);
                            continue;
                        }
                        var jsonStr = elem.ToString();
                        if (string.IsNullOrEmpty(jsonStr))
                        {
                            missingKeys.Add(key);
                            continue;
                        }
                        var val = JsonConvert.DeserializeObject<TModel>(jsonStr);
                        if (val == null || val.Deleted)
                        {
                            missingKeys.Add(key);
                            continue;
                        }
                        result.Add(val);
                        _local[redisKeys[i]] = jsonStr;
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
                    await using (await _distributedLock.Lock(world, GetLockKey(key)))
                    {
                        var hash = key.GetHash();
                        var redisForKey = hash == null ? _redisService.GetGlobalConnection(world) : _redisService.GetShardConnection(world, hash.Value);
                        if (redisForKey != null)
                        {
                            if (!_updateValueExpiryScripts.TryGetValue(redisForKey, out var script))
                            {
                                script = LuaScript.Prepare(UpdateValueExpiryScript).Load(redisForKey.GetServer());
                                _updateValueExpiryScripts[redisForKey] = script;
                            }

                            await redisForKey.Connection.ScriptEvaluateAsync(script, new
                            {
                                key = key.GetRedisKey(),
                                value = JsonConvert.SerializeObject(firstEntity),
                                cref = new RedisKey(Const.ReferenceCountKey),
                                expiry = (int)Const.CacheTimeToLive.TotalSeconds
                            });
                            _local[key.GetRedisKey()] = JsonConvert.SerializeObject(firstEntity);
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
}
