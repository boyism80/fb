using Http.Model;
using Http.Redis;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository.Cache
{
    /// <summary>
    /// L2 Redis cache layer for single-key (value) entities.
    /// </summary>
    public class RedisValueCacheLayer<TModel> where TModel : class, IModel
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

        private readonly RedisService _redisService;
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _updateValueExpiryScripts = new Dictionary<Service.Redis, LoadedLuaScript>();
        private readonly Dictionary<Service.Redis, LoadedLuaScript> _multiValueGetScripts = new Dictionary<Service.Redis, LoadedLuaScript>();

        public RedisValueCacheLayer(RedisService redisService)
        {
            _redisService = redisService;
        }

        public Service.Redis GetConnection(uint world, uint? hash)
        {
            return hash == null
                ? _redisService.GetGlobalConnection(world)
                : _redisService.GetShardConnection(world, hash.Value);
        }

        public async Task<TModel> TryGetAsync(Service.Redis redis, RedisKey redisKey)
        {
            if (redis == null)
                return null;

            var value = await redis.Connection.JsonGetAsync<TModel>(redisKey);
            if (value == null || value.Deleted)
                return null;

            return value;
        }

        public async Task WriteBackAsync(Service.Redis redis, RedisKey redisKey, TModel value)
        {
            if (redis == null)
                return;

            if (!_updateValueExpiryScripts.TryGetValue(redis, out var script))
            {
                script = LuaScript.Prepare(UpdateValueExpiryScript).Load(redis.GetServer());
                _updateValueExpiryScripts[redis] = script;
            }

            await redis.Connection.ScriptEvaluateAsync(script, new
            {
                key = redisKey,
                value = JsonConvert.SerializeObject(value),
                cref = new RedisKey(Const.ReferenceCountKey),
                expiry = (int)Const.CacheTimeToLive.TotalSeconds
            });
        }

        public async Task SetAsync(Service.Redis redis, RedisKey redisKey, TModel value)
        {
            if (redis == null)
                return;

            await redis.Connection.TransactAsync(cmd =>
            {
                cmd.Enqueue(trans => trans.JsonSetAsync(redisKey, value));
                cmd.Enqueue(trans => trans.KeyExpireAsync(redisKey, expiry: (TimeSpan?)null));
                cmd.Enqueue(trans => trans.HashIncrementAsync(Const.ReferenceCountKey, redisKey.ToString()));
            });
        }

        public async Task<IReadOnlyList<(RedisKey RedisKey, string Json)>> TryGetManyAsync(
            Service.Redis redis,
            IReadOnlyList<RedisKey> redisKeys)
        {
            var result = new List<(RedisKey, string)>();
            if (redis == null || redisKeys.Count == 0)
                return result;

            var keys = new RedisKey[redisKeys.Count + 1];
            for (var i = 0; i < redisKeys.Count; i++)
            {
                keys[i] = redisKeys[i];
            }
            keys[redisKeys.Count] = Const.ReferenceCountKey;
            var values = new RedisValue[] { (int)Const.CacheTimeToLive.TotalSeconds };

            if (!_multiValueGetScripts.TryGetValue(redis, out var multiGetScript))
            {
                multiGetScript = LuaScript.Prepare(MultiValueGetScript).Load(redis.GetServer());
                _multiValueGetScripts[redis] = multiGetScript;
            }

            var multiResult = await redis.Connection.ScriptEvaluateAsync(multiGetScript.Hash, keys, values);
            var elements = (RedisResult[])multiResult;
            for (var i = 0; i < redisKeys.Count; i++)
            {
                var elem = elements[i];
                if (elem.IsNull)
                    continue;

                var jsonStr = elem.ToString();
                if (string.IsNullOrEmpty(jsonStr))
                    continue;

                result.Add((redisKeys[i], jsonStr));
            }

            return result;
        }
    }
}
