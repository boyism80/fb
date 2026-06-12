using Http.Model;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository.Cache
{
    /// <summary>
    /// L1 in-process cache for Redis value (single-key) entities.
    /// Scoped to the repository instance (one per HTTP request).
    /// </summary>
    public class LocalValueMemoryCacheLayer<TModel> where TModel : class, IModel
    {
        private readonly Dictionary<string, string> _cache = new Dictionary<string, string>();

        private static string ToKey(RedisKey redisKey) => redisKey.ToString();

        public TModel TryGet(RedisKey redisKey)
        {
            if (!_cache.TryGetValue(ToKey(redisKey), out var json))
                return null;

            var value = JsonConvert.DeserializeObject<TModel>(json);
            if (value == null || value.Deleted)
                return null;

            return value;
        }

        public bool TryGetRaw(RedisKey redisKey, out string json)
        {
            return _cache.TryGetValue(ToKey(redisKey), out json);
        }

        public void Put(RedisKey redisKey, TModel value)
        {
            _cache[ToKey(redisKey)] = JsonConvert.SerializeObject(value);
        }

        public void PutRaw(RedisKey redisKey, string json)
        {
            _cache[ToKey(redisKey)] = json;
        }
    }
}
