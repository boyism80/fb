using StackExchange.Redis;
using StackExchange.Redis.Extensions.Core.Configuration;

namespace http.Service
{
    public class RedisService
    {
        private readonly RedisConfiguration _configuration;
        private static Dictionary<string, LoadedLuaScript> _loadedLuaScripts = new Dictionary<string, LoadedLuaScript>();

        public RedisService(IConfiguration configuration)
        {
            _configuration = configuration.GetSection("Redis").Get<RedisConfiguration>();
            LoadScriptFiles(Path.Combine("Redis", "Script"));
        }

        private void LoadScriptFiles(string path)
        {
            if (!Directory.Exists(path))
                return;

            var redis = ConnectionMultiplexer.Connect(_configuration.ConfigurationOptions);
            foreach (var file in Directory.GetFiles(path, "*.lua"))
            {
                var script = LuaScript.Prepare(File.ReadAllText(file));
                var loadedScript = script.Load(redis.GetServer(redis.GetEndPoints()[0]));
                _loadedLuaScripts.Add(Path.GetFileName(file), loadedScript);
            }
        }

        public static LoadedLuaScript GetLoadedLuaScript(string file)
        {
            return _loadedLuaScripts.GetValueOrDefault(file);
        }

        public IDatabase Connection
        {
            get
            {
                var redis = ConnectionMultiplexer.Connect(_configuration.ConfigurationOptions);
                return redis.GetDatabase(_configuration.Database);
            }
        }
    }
}
