using System.Collections.Concurrent;
using StackExchange.Redis;

namespace Http.Redis
{
    internal sealed class RedisScriptRunner
    {
        private readonly ConcurrentDictionary<string, LuaScript> _prepared = new();
        private readonly ConcurrentDictionary<(Service.Redis Redis, string Source), LoadedLuaScript> _loaded = new();

        public Task<RedisResult> EvalAsync(
            Service.Redis redis,
            string source,
            RedisKey[] keys,
            RedisValue[] values = null)
        {
            var loaded = GetLoaded(redis, source);
            return redis.Connection.ScriptEvaluateAsync(loaded.Hash, keys, values);
        }

        public Task<RedisResult> EvalAsync(
            Service.Redis redis,
            string source,
            object parameters)
        {
            var loaded = GetLoaded(redis, source);
            return redis.Connection.ScriptEvaluateAsync(loaded, parameters);
        }

        private LoadedLuaScript GetLoaded(Service.Redis redis, string source)
        {
            var prepared = _prepared.GetOrAdd(source, LuaScript.Prepare);
            return _loaded.GetOrAdd((redis, source), _ => prepared.Load(redis.GetServer()));
        }
    }
}
