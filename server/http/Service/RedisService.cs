using StackExchange.Redis;
using StackExchange.Redis.Extensions.Core.Configuration;
using System.Text;

namespace Http.Service
{
    public class Redis
    {
        private readonly ConnectionMultiplexer _redis;
        private readonly Dictionary<string, LoadedLuaScript> _loadedLuaScripts = new Dictionary<string, LoadedLuaScript>();

        public Redis(RedisHost host)
        {
            _redis = ConnectionMultiplexer.Connect($"{host.Host}:{host.Port},abortConnect=false,connectTimeout=30000,responseTimeout=30000");

            LoadScriptFiles(Path.Combine("Redis", "Script"));
        }

        private void LoadScriptFiles(string path)
        {
            if (!Directory.Exists(path))
                return;

            foreach (var file in Directory.GetFiles(path, "*.lua"))
            {
                var fileName = Path.GetFileName(file);
                var script = LuaScript.Prepare(File.ReadAllText(file));
                var loadedScript = script.Load(_redis.GetServer(_redis.GetEndPoints()[0]));
                _loadedLuaScripts.Add(fileName, loadedScript);
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
                return _redis.GetDatabase(0);
            }
        }

        public IServer GetServer()
        {
            return _redis.GetServer(_redis.GetEndPoints()[0]);
        }
    }

    public class RedisService
    {
        private Redis _unifiedRedis;
        private readonly Dictionary<uint, Redis> _worldGlobalRedis = new Dictionary<uint, Redis>();
        private readonly Dictionary<uint, List<Redis>> _worldShardRedis = new Dictionary<uint, List<Redis>>();
        private readonly IConfiguration _configuration;

        public RedisService(IConfiguration configuration)
        {
            _configuration = configuration;

            // Load unified Redis
            var unifiedHost = _configuration.GetSection("Redis:unified").Get<RedisHost>();
            if (unifiedHost != null)
            {
                _unifiedRedis = new Redis(unifiedHost);
            }

            // Load world-specific Redis
            var worldsSection = _configuration.GetSection("Redis:worlds");
            foreach (var worldChild in worldsSection.GetChildren())
            {
                if (uint.TryParse(worldChild.Key, out var world))
                {
                    // Load global Redis for this world
                    var globalHost = worldChild.GetSection("global").Get<RedisHost>();
                    if (globalHost != null)
                    {
                        _worldGlobalRedis[world] = new Redis(globalHost);
                    }

                    // Load shard Redis array for this world
                    var dataArray = worldChild.GetSection("data").Get<RedisHost[]>();
                    if (dataArray != null && dataArray.Length > 0)
                    {
                        var shardList = dataArray.Select(host => new Redis(host)).ToList();
                        _worldShardRedis[world] = shardList;
                    }
                }
            }
        }

        public Redis GetUnifiedConnection()
        {
            return _unifiedRedis;
        }

        public Redis GetGlobalConnection(uint world)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Use GetUnifiedConnection() for unified-global.");

            return _worldGlobalRedis.TryGetValue(world, out var redis) ? redis : null;
        }

        public Redis GetDataConnection(uint world, int index)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Use GetUnifiedConnection() for unified-global.");

            if (index < 0)
                return GetGlobalConnection(world);

            if (!_worldShardRedis.TryGetValue(world, out var shardList))
                return null;

            if (index >= shardList.Count)
                return null;

            return shardList[index];
        }

        public Redis GetShardConnection(uint world, uint id)
        {
            var shardSize = GetShardSize(world);
            if (shardSize == 0)
                return null;

            var index = (int)(id % (uint)shardSize);
            return GetDataConnection(world, index);
        }

        public Redis GetShardConnection(uint world, string key)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Use GetUnifiedConnection() for unified-global.");

            var shardSize = GetShardSize(world);
            if (shardSize == 0)
                return null;

            ulong hash = 0;
            foreach (var b in Encoding.UTF8.GetBytes(key))
            {
                hash = hash * 31 + b;
            }
            var shardIndex = (int)(hash % (ulong)shardSize);
            return GetDataConnection(world, shardIndex);
        }

        public int GetShardSize(uint world)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Unified has no shards.");

            return _worldShardRedis.TryGetValue(world, out var shardList) ? shardList.Count : 0;
        }

        public IReadOnlyList<uint> GetConfiguredWorlds()
        {
            var worlds = new List<uint>(_worldGlobalRedis.Keys);
            worlds.Sort();
            return worlds;
        }
    }
}