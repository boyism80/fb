using http.Service;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using StackExchange.Redis;

namespace http.Redis
{
    public static class Extension
    {
        public static async Task<RedisResult> ScriptEvaluateAsync(this IDatabase database, string file, object param = null)
        {
            var loadedScript = RedisService.GetLoadedLuaScript(file) ??
                throw new NullReferenceException();

            return await database.ScriptEvaluateAsync(loadedScript, param);
        }

        public static async Task<T> JsonGetAsync<T>(this IDatabase database, RedisKey key) where T : class
        {
            var value = await database.StringGetAsync(key);
            if (value.IsNull)
                return null;

            return JsonConvert.DeserializeObject<T>(value);
        }

        public static async Task<bool> JsonSetAsync<T>(this IDatabase database, RedisKey key, T value, TimeSpan? expiry = null) where T : class
        {
            return await database.StringSetAsync(key, JsonConvert.SerializeObject(value), expiry);
        }

        public static async Task<T> GetAsync<T>(this IDatabase database, RedisKey key) where T : struct
        {
            var value = await database.StringGetAsync(key);
            if (value.IsNull)
                throw new KeyNotFoundException();

            return new JValue(value).ToObject<T>();
        }

        public static async Task<bool> SetAsync<T>(this IDatabase database, RedisKey key, T value, TimeSpan? expiry = null) where T : struct
        {
            return await database.StringSetAsync(key, new JValue(value).ToString(), expiry);
        }
    }
}
