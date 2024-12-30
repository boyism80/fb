using Http.Model;
using StackExchange.Redis;
using StackExchange.Redis.Extensions.Core.Configuration;
using System.Text;

namespace Http.Service
{
    public class Redis
    {
        private readonly RedisConfiguration _conf;
        private readonly ConnectionMultiplexer _redis;
        private readonly Dictionary<string, LoadedLuaScript> _loadedLuaScripts = new Dictionary<string, LoadedLuaScript>();

        public Redis(RedisConfiguration conf)
        {
            _conf = conf;
            _redis = ConnectionMultiplexer.Connect(conf.ConfigurationOptions);

            LoadScriptFiles(Path.Combine("Redis", "Script"));
        }

        private void LoadScriptFiles(string path)
        {
            if (!Directory.Exists(path))
                return;

            foreach (var file in Directory.GetFiles(path, "*.lua"))
            {
                var script = LuaScript.Prepare(File.ReadAllText(file));
                var loadedScript = script.Load(_redis.GetServer(_redis.GetEndPoints()[0]));
                _loadedLuaScripts.Add(Path.GetFileName(file), loadedScript);
            }
        }

        public LoadedLuaScript GetLoadedLuaScript(string file)
        {
            return _loadedLuaScripts.GetValueOrDefault(file);
        }

        public IDatabase Connection
        {
            get
            {
                return _redis.GetDatabase(_conf.Database);
            }
        }
    }

    public class RedisService
    {
        private readonly RedisConfiguration _configuration;
        private readonly Dictionary<int, Redis> _redis = new Dictionary<int, Redis>();
        public int ShardSize { get; private set; }

        public RedisService(IConfiguration configuration)
        {
            var size = 0;
            foreach (var section in configuration.GetSection("Redis").GetChildren())
            {
                var id = int.Parse(section.Key);
                var redisConf = section.Get<RedisConfiguration>();
                _redis.Add(id, new Redis(redisConf));

                if (id != -1)
                    size++;
            }

            ShardSize = size;
        }

        public Redis Redis(int id)
        {
            if (_redis.ContainsKey(id) == false)
                return null;

            return _redis[id];
        }

        public Redis Redis(uint id)
        {
            return Redis((int)(id % ShardSize));
        }

        public Redis Redis(string key)
        {
            ulong hash = 0;
            foreach (var b in Encoding.UTF8.GetBytes(key))
            {
                hash = hash * 31 + b;
            }

            return Redis((int)(hash % (ulong)ShardSize));
        }

        public Redis Redis(IRedisValueKey key)
        {
            return Redis(key.GetHash());
        }

        public Redis Redis(RedisKey key)
        {
            return Redis(key.ToString());
        }
    }
}
