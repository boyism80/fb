using Http.Model;
using Http.Redis;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository.Cache
{
    public enum RedisCacheLookupStatus
    {
        Miss = 0,
        Hit = 1,
        Pending = 2,
    }

    /// <summary>
    /// L2 Redis cache layer for single-key (value) entities.
    /// </summary>
    public class RedisValueCacheLayer<TModel> where TModel : class, IModel
    {
        private static readonly string LookupValueScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]

            local value = redis.call('GET', CACHE_KEY)
            if value and value ~= '' then
                return {1, value}
            end

            if redis.call('HEXISTS', COUNT_REFS, CACHE_KEY) == 1 then
                return {2}
            end

            return {0}
            """;

        private static readonly string UpdateValueExpiryScript = """
            local CACHE_KEY = @key
            local COUNT_REFS = @cref
            local EXPIRY = tonumber(@expiry)

            if redis.call('hexists', COUNT_REFS, CACHE_KEY) == 1 then
                return 0
            end

            redis.call('set', CACHE_KEY, @value)

            local contains_refs = redis.call('hexists', COUNT_REFS, CACHE_KEY)
            if contains_refs == 0 then
                redis.call('expire', CACHE_KEY, EXPIRY)
            end

            return 1
            """;

        private static readonly string RemovePendingWriteBackScript = """
            local CACHE_KEY = KEYS[1]
            local COUNT_REFS = KEYS[2]

            redis.call('hincrby', COUNT_REFS, CACHE_KEY, 1)
            redis.call('del', CACHE_KEY)

            return 1
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

        public async Task<(RedisCacheLookupStatus Status, TModel Value)> LookupAsync(Service.Redis redis, RedisKey redisKey)
        {
            if (redis == null)
                return (RedisCacheLookupStatus.Miss, null);

            var result = (RedisResult[])await redis.EvalAsync(
                LookupValueScript,
                [redisKey, new RedisKey(Const.ReferenceCountKey)]);

            var status = (RedisCacheLookupStatus)int.Parse(result[0].ToString());
            if (status != RedisCacheLookupStatus.Hit)
                return (status, null);

            return (status, JsonConvert.DeserializeObject<TModel>(result[1].ToString()));
        }

        public async Task<bool> WriteBackAsync(Service.Redis redis, RedisKey redisKey, TModel value)
        {
            if (redis == null)
                return false;

            var result = await redis.EvalAsync(UpdateValueExpiryScript, new
            {
                key = redisKey,
                value = JsonConvert.SerializeObject(value),
                cref = new RedisKey(Const.ReferenceCountKey),
                expiry = (int)Const.CacheTimeToLive.TotalSeconds
            });

            return int.Parse(result.ToString()) != 0;
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

        public async Task RemovePendingWriteBackAsync(Service.Redis redis, RedisKey redisKey)
        {
            if (redis == null)
                return;

            await redis.EvalAsync(
                RemovePendingWriteBackScript,
                [redisKey, new RedisKey(Const.ReferenceCountKey)]);
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

            var multiResult = await redis.EvalAsync(MultiValueGetScript, keys, values);
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
