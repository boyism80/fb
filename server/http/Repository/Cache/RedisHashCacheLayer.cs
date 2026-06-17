using Http.Model;
using Http.Redis;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository.Cache
{
    /// <summary>
    /// L2 Redis cache layer for hash (multi-field) entities.
    /// </summary>
    public class RedisHashCacheLayer<TModel> where TModel : class, IModel, IRedisHashKey
    {
        private static readonly string UpdateHashExpiryScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]
            local EXPIRY = tonumber(ARGV[1])
            local LENGTH = tonumber(ARGV[2])

            if redis.call('hexists', COUNT_REFS, CACHE_KEY) == 1 then
                return {0}
            end

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

            return {1}
            """;

        private static readonly string SetHashFieldScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]
            local FIELD = ARGV[1]
            local VALUE = ARGV[2]

            redis.call('hset', CACHE_KEY, FIELD, VALUE)
            redis.call('persist', CACHE_KEY)
            redis.call('hincrby', COUNT_REFS, CACHE_KEY, 1)

            return 1
            """;

        private static readonly string SetHashFieldsScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]
            local LENGTH = tonumber(ARGV[1])

            for i = 1, LENGTH do
                local field = ARGV[i * 2]
                local value = ARGV[i * 2 + 1]
                redis.call('hset', CACHE_KEY, field, value)
            end

            redis.call('persist', CACHE_KEY)
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

        private static readonly string LookupHashKeyScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]

            if redis.call('EXISTS', CACHE_KEY) == 1 then
                return 1
            end

            if redis.call('HEXISTS', COUNT_REFS, CACHE_KEY) == 1 then
                return 2
            end

            return 0
            """;

        private static readonly string RemoveFieldsPendingWriteBackScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]
            local LENGTH = tonumber(ARGV[1])

            redis.call('hincrby', COUNT_REFS, CACHE_KEY, 1)

            for i = 1, LENGTH do
                redis.call('hdel', CACHE_KEY, ARGV[i + 1])
            end

            return 1
            """;

        private readonly RedisService _redisService;

        public RedisHashCacheLayer(RedisService redisService)
        {
            _redisService = redisService;
        }

        public Service.Redis GetConnection(uint world, uint? hash)
        {
            return hash == null
                ? _redisService.GetGlobalConnection(world)
                : _redisService.GetShardConnection(world, hash.Value);
        }

        public async Task<IReadOnlyDictionary<RedisValue, TModel>> TryGetAllAsync(Service.Redis redis, RedisKey redisKey)
        {
            if (redis == null)
                return new Dictionary<RedisValue, TModel>();

            return await redis.Connection.JsonHashGetAllAsync<TModel>(redisKey);
        }

        public async Task<IReadOnlyDictionary<RedisValue, TModel>> TryGetFieldsAsync(Service.Redis redis, RedisKey redisKey)
        {
            if (redis == null)
                return new Dictionary<RedisValue, TModel>();

            return await redis.Connection.JsonHashGetAsync<TModel>(redisKey);
        }

        public async Task<bool> KeyExistsAsync(Service.Redis redis, RedisKey redisKey)
        {
            if (redis == null)
                return false;

            return await redis.Connection.KeyExistsAsync(redisKey);
        }

        public async Task<RedisCacheLookupStatus> LookupHashKeyAsync(Service.Redis redis, RedisKey redisKey)
        {
            if (redis == null)
                return RedisCacheLookupStatus.Miss;

            var result = await redis.EvalAsync(
                LookupHashKeyScript,
                [redisKey, new RedisKey(Const.ReferenceCountKey)]);

            return (RedisCacheLookupStatus)int.Parse(result.ToString());
        }

        public async Task<bool> WriteBackAsync(Service.Redis redis, RedisKey cacheKey, IEnumerable<TModel> values)
        {
            if (redis == null)
                return false;

            var dataGroup = values.ToDictionary(x => x.GetRedisField(), x => x);
            if (dataGroup.Count == 0)
                return false;

            var scriptValues = new List<RedisValue>
            {
                (int)Const.CacheTimeToLive.TotalSeconds,
                dataGroup.Count
            };
            foreach (var (field, entity) in dataGroup)
            {
                scriptValues.Add(field);
                scriptValues.Add(JsonConvert.SerializeObject(entity));
            }

            var result = (RedisResult[])await redis.EvalAsync(
                UpdateHashExpiryScript,
                [cacheKey, new RedisKey(Const.ReferenceCountKey)],
                scriptValues.ToArray());

            return int.Parse(result[0].ToString()) != 0;
        }

        public async Task SetFieldAsync(Service.Redis redis, RedisKey cacheKey, RedisValue field, TModel value)
        {
            if (redis == null)
                return;

            await redis.EvalAsync(
                SetHashFieldScript,
                [cacheKey, new RedisKey(Const.ReferenceCountKey)],
                [field, JsonConvert.SerializeObject(value)]);
        }

        public async Task SetFieldsAsync(Service.Redis redis, RedisKey cacheKey, IReadOnlyDictionary<RedisValue, TModel> values)
        {
            if (redis == null || values.Count == 0)
                return;

            var scriptValues = new List<RedisValue> { values.Count };
            foreach (var (field, val) in values)
            {
                scriptValues.Add(field);
                scriptValues.Add(JsonConvert.SerializeObject(val));
            }

            await redis.EvalAsync(
                SetHashFieldsScript,
                [cacheKey, new RedisKey(Const.ReferenceCountKey)],
                scriptValues.ToArray());
        }

        public async Task RemoveFieldsPendingWriteBackAsync(Service.Redis redis, RedisKey cacheKey, IReadOnlyList<RedisValue> fields)
        {
            if (redis == null || fields == null || fields.Count == 0)
                return;

            var scriptValues = new List<RedisValue> { fields.Count };
            scriptValues.AddRange(fields);

            await redis.EvalAsync(
                RemoveFieldsPendingWriteBackScript,
                [cacheKey, new RedisKey(Const.ReferenceCountKey)],
                scriptValues.ToArray());
        }

        public async Task<IReadOnlyList<(RedisKey RedisKey, Dictionary<string, string> Fields)>> TryGetManyAsync(
            Service.Redis redis,
            IReadOnlyList<RedisKey> redisKeys)
        {
            var result = new List<(RedisKey, Dictionary<string, string>)>();
            if (redis == null || redisKeys.Count == 0)
                return result;

            var keys = new RedisKey[redisKeys.Count + 1];
            for (var i = 0; i < redisKeys.Count; i++)
            {
                keys[i] = redisKeys[i];
            }
            keys[redisKeys.Count] = Const.ReferenceCountKey;
            var values = new RedisValue[] { (int)Const.CacheTimeToLive.TotalSeconds };

            var multiResult = await redis.EvalAsync(MultiHashGetScript, keys, values);
            var flat = (RedisResult[])multiResult;
            var idx = 0;
            for (var i = 0; i < redisKeys.Count; i++)
            {
                if (idx >= flat.Length)
                    break;

                idx++;
                var count = int.Parse(flat[idx++].ToString());
                if (count == 0)
                    continue;

                var localCache = new Dictionary<string, string>();
                for (var c = 0; c < count; c++)
                {
                    var field = flat[idx++].ToString();
                    var valStr = flat[idx++].ToString();
                    localCache[field] = valStr;
                }

                result.Add((redisKeys[i], localCache));
            }

            return result;
        }
    }
}
