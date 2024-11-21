using Http.Service;
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
        public static async Task<RedisResult> ScriptEvaluateAsync(this IDatabaseAsync database, string file, object param = null)
        {
            var loadedScript = RedisService.GetLoadedLuaScript(file) ??
                throw new NullReferenceException();

            return await database.ScriptEvaluateAsync(loadedScript, param);
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

        private static async Task<bool> Lock<T>(this IDatabaseAsync database, string key, TaskCompletionSource<T> tcs, Func<Task<T>> fn, string uuid, ISubscriber sub)
        {
            var success = await database.ScriptEvaluateAsync("redis_lock.lua", new
            {
                key = key,
                expiry = (int)TimeSpan.FromSeconds(5).TotalSeconds
            });
            if ((bool)success)
            {
                try
                {
                    var result = await fn();
                    await database.KeyDeleteAsync(key);
                    await sub.UnsubscribeAsync(key);
                    await database.PublishAsync(key, uuid);
                    tcs.SetResult(result);
                    return true;
                }
                catch (Exception e)
                {
                    tcs.SetException(e);
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        public static async Task<T> Sync<T>(this IDatabaseAsync database, string key, Func<Task<T>> fn)
        {
            var tcs = new TaskCompletionSource<T>();
            var uuid = Guid.NewGuid().ToString();
            var sub = database.Multiplexer.GetSubscriber();
            var channel = await sub.SubscribeAsync(key);
            channel.OnMessage(async message =>
            {
                if (message.Channel != key)
                    return;

                if (message.Message != uuid)
                    await database.Lock(key, tcs, fn, uuid, sub);
            });

            await database.Lock(key, tcs, fn, uuid, sub);
            return await tcs.Task;
        }
    }
}
