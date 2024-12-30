using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using StackExchange.Redis;

namespace Http.Redis
{
    public class RedisCommandQueue
    {
        private readonly ITransaction _tran;
        private readonly IList<Task> _tasks = new List<Task>();

        public RedisCommandQueue Enqueue(Func<ITransaction, Task> cmd)
        {
            _tasks.Add(cmd(_tran));
            return this;
        }

        internal RedisCommandQueue(ITransaction tran) => _tran = tran;
        internal Task CompleteAsync() => Task.WhenAll(_tasks);
    }

    public static class Extension
    {
        public static async Task<RedisResult> ScriptEvaluateAsync(this Http.Service.Redis redis, string file, object param = null)
        {
            var loadedScript = redis.GetLoadedLuaScript(file) ??
                throw new NullReferenceException();

            return await redis.Connection.ScriptEvaluateAsync(loadedScript, param);
        }

        public static async Task<RedisResult> ScriptEvaluateAsync(this Http.Service.Redis redis, string file, RedisKey[] keys = null, RedisValue[] values = null)
        {
            var loadedScript = redis.GetLoadedLuaScript(file) ??
                throw new NullReferenceException();

            return await redis.Connection.ScriptEvaluateAsync(loadedScript.Hash, keys, values);
        }

        public static async Task<T> JsonGetAsync<T>(this IDatabaseAsync database, RedisKey key) where T : class
        {
            var value = await database.StringGetAsync(key);
            if (value.IsNull)
                return null;

            return JsonConvert.DeserializeObject<T>(value);
        }

        public static async Task<T> JsonHashGetAsync<T>(this IDatabaseAsync database, RedisKey key, RedisValue field) where T : class
        {
            var value = await database.HashGetAsync(key, field);
            if (value.IsNull)
                return null;

            return JsonConvert.DeserializeObject<T>(value.ToString());
        }

        public static async Task<IReadOnlyDictionary<RedisValue, T>> JsonHashGetAllAsync<T>(this IDatabaseAsync database, RedisKey key) where T : class
        {
            var values = await database.HashGetAllAsync(key);
            return values.ToDictionary(x => x.Name, x => JsonConvert.DeserializeObject<T>(x.Value));
        }

        public static async Task<IReadOnlyDictionary<RedisValue, T>> JsonHashGetAsync<T>(this IDatabaseAsync database, RedisKey key) where T : class
        {
            var value = await database.HashGetAllAsync(key);
            return value.ToDictionary(x => x.Name, x => JsonConvert.DeserializeObject<T>(x.Value));
        }

        public static async Task<bool> JsonSetAsync<T>(this IDatabaseAsync database, RedisKey key, T value, TimeSpan? expiry = null) where T : class
        {
            return await database.StringSetAsync(key, JsonConvert.SerializeObject(value), expiry);
        }

        public static async Task JsonHashSetAsync<T>(this IDatabaseAsync database, RedisKey key, RedisValue field, T value) where T : class
        {
            await database.HashSetAsync(key, field, JsonConvert.SerializeObject(value));
        }

        public static async Task JsonHashSetAsync<T>(this IDatabaseAsync database, RedisKey key, IReadOnlyDictionary<RedisValue, T> values) where T : class
        {
            var hashFields = values.Select(x =>
            {
                var field = x.Key;
                var value = JsonConvert.SerializeObject(x.Value);
                return new HashEntry(field, value);
            }).ToArray();
            await database.HashSetAsync(key, hashFields);
        }

        public static async Task<T> GetAsync<T>(this IDatabaseAsync database, RedisKey key) where T : struct
        {
            var value = await database.StringGetAsync(key);
            if (value.IsNull)
                throw new KeyNotFoundException();

            return new JValue(value).ToObject<T>();
        }

        public static async Task<bool> SetAsync<T>(this IDatabaseAsync database, RedisKey key, T value, TimeSpan? expiry = null) where T : struct
        {
            return await database.StringSetAsync(key, new JValue(value).ToString(), expiry);
        }

        public static async Task TransactAsync(this IDatabase db, Action<RedisCommandQueue> addCommands)
        {
            var tran = db.CreateTransaction();
            var q = new RedisCommandQueue(tran);

            addCommands(q);

            if (await tran.ExecuteAsync())
                await q.CompleteAsync();
        }
    }
}
