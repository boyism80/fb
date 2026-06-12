using Http.Model;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Reepository.Cache
{
    /// <summary>
    /// L1 in-process cache for Redis hash (multi-field) entities.
    /// Scoped to the repository instance (one per HTTP request).
    /// </summary>
    public class LocalHashMemoryCacheLayer<TModel> where TModel : class, IModel
    {
        private readonly Dictionary<string, Dictionary<string, string>> _cache = new Dictionary<string, Dictionary<string, string>>();

        private static string ToKey(RedisKey redisKey) => redisKey.ToString();

        public TModel TryGetField(RedisKey redisKey, RedisValue field)
        {
            if (!_cache.TryGetValue(ToKey(redisKey), out var fields))
                return null;

            if (!fields.TryGetValue(field.ToString(), out var json))
                return null;

            var value = JsonConvert.DeserializeObject<TModel>(json);
            if (value == null || value.Deleted)
                return null;

            return value;
        }

        public bool HasKey(RedisKey redisKey)
        {
            return _cache.ContainsKey(ToKey(redisKey));
        }

        public IEnumerable<TModel> GetAll(RedisKey redisKey)
        {
            if (!_cache.TryGetValue(ToKey(redisKey), out var fields))
                return Enumerable.Empty<TModel>();

            return fields.Values
                .Select(x => JsonConvert.DeserializeObject<TModel>(x))
                .Where(x => x != null && !x.Deleted);
        }

        public void PutAll(RedisKey redisKey, Dictionary<string, string> fields)
        {
            _cache[ToKey(redisKey)] = new Dictionary<string, string>(fields);
        }

        public void PutField(RedisKey redisKey, RedisValue field, string json)
        {
            var key = ToKey(redisKey);
            if (!_cache.TryGetValue(key, out var fields))
            {
                fields = new Dictionary<string, string>();
                _cache[key] = fields;
            }

            fields[field.ToString()] = json;
        }

        public void PutField(RedisKey redisKey, RedisValue field, TModel value)
        {
            PutField(redisKey, field, JsonConvert.SerializeObject(value));
        }

        public void Remove(RedisKey redisKey)
        {
            _cache.Remove(ToKey(redisKey));
        }
    }
}
