using StackExchange.Redis;
using StackExchange.Redis.Extensions.Core.Configuration;
using System.Text;

namespace Http.Service
{
    /// <summary>
    /// Represents a Redis connection wrapper that manages database connections and Lua scripts.
    /// Provides functionality for loading and executing Lua scripts on Redis server.
    /// </summary>
    public class Redis
    {
        private readonly ConnectionMultiplexer _redis;
        private readonly Dictionary<string, LoadedLuaScript> _loadedLuaScripts = new Dictionary<string, LoadedLuaScript>();

        /// <summary>
        /// Initializes a new instance of the <see cref="Redis"/> class.
        /// Establishes connection to Redis server and loads Lua scripts from code and files.
        /// </summary>
        /// <param name="host">The Redis host configuration containing connection details.</param>
        public Redis(RedisHost host)
        {
            _redis = ConnectionMultiplexer.Connect($"{host.Host}:{host.Port},abortConnect=false,connectTimeout=30000,responseTimeout=30000");

            LoadScriptFiles(Path.Combine("Redis", "Script"));
        }

        /// <summary>
        /// Loads all Lua script files from the specified directory path.
        /// Scripts are preloaded on the Redis server for efficient execution.
        /// </summary>
        /// <param name="path">The directory path containing Lua script files.</param>
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

        /// <summary>
        /// Retrieves a preloaded Lua script by filename.
        /// </summary>
        /// <param name="file">The filename of the Lua script to retrieve.</param>
        /// <returns>The loaded Lua script instance, or null if not found.</returns>
        public LoadedLuaScript GetLoadedLuaScript(string file)
        {
            return _loadedLuaScripts.GetValueOrDefault(file);
        }

        /// <summary>
        /// Gets the Redis database connection instance.
        /// </summary>
        /// <value>The Redis database instance for database 0.</value>
        public IDatabase Connection
        {
            get
            {
                return _redis.GetDatabase(0);
            }
        }

        /// <summary>
        /// Gets the Redis server instance for script loading.
        /// </summary>
        /// <returns>The Redis server instance.</returns>
        public IServer GetServer()
        {
            return _redis.GetServer(_redis.GetEndPoints()[0]);
        }
    }

    /// <summary>
    /// Provides Redis service management with sharding support.
    /// Manages multiple Redis instances and provides routing logic for distributed operations.
    /// </summary>
    public class RedisService
    {
        private Redis _unifiedRedis;
        private readonly Dictionary<uint, Redis> _worldGlobalRedis = new Dictionary<uint, Redis>();
        private readonly Dictionary<uint, List<Redis>> _worldShardRedis = new Dictionary<uint, List<Redis>>();
        private readonly IConfiguration _configuration;

        /// <summary>
        /// Initializes a new instance of the <see cref="RedisService"/> class.
        /// Creates Redis connections using unified/global/data structure: Redis:{unified|worlds:{world}:{global|data}}.
        /// </summary>
        /// <param name="configuration">The application configuration containing Redis connection settings.</param>
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

        /// <summary>
        /// Gets unified-global Redis instance.
        /// Use this for services that need cross-world data.
        /// Unified Redis is shared across all worlds (no world parameter needed).
        /// </summary>
        /// <returns>The unified-global Redis instance, or null if not found.</returns>
        public Redis GetUnifiedConnection()
        {
            return _unifiedRedis;
        }

        /// <summary>
        /// Gets world-global Redis instance for the specified world.
        /// Use this for world-specific global data.
        /// </summary>
        /// <param name="world">The world identifier (must be > 0).</param>
        /// <returns>The world-global Redis instance, or null if not found.</returns>
        public Redis GetGlobalConnection(uint world)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Use GetUnifiedConnection() for unified-global.");

            return _worldGlobalRedis.TryGetValue(world, out var redis) ? redis : null;
        }

        /// <summary>
        /// Gets shard Redis instance for the specified world and shard index.
        /// Use this for sharded data.
        /// </summary>
        /// <param name="world">The world identifier (must be > 0).</param>
        /// <param name="index">The shard index (0-based array index).</param>
        /// <returns>The shard Redis instance, or null if not found.</returns>
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

        /// <summary>
        /// Gets shard Redis instance using ID-based sharding.
        /// Automatically calculates shard index from ID using modulo operation.
        /// </summary>
        /// <param name="world">The world identifier (must be > 0).</param>
        /// <param name="id">The ID used for sharding calculation.</param>
        /// <returns>The shard Redis instance, or null if not found.</returns>
        public Redis GetShardConnection(uint world, uint id)
        {
            var shardSize = GetShardSize(world);
            if (shardSize == 0)
                return null;

            var index = (int)(id % (uint)shardSize);
            return GetDataConnection(world, index);
        }

        /// <summary>
        /// Gets shard Redis instance using string key-based sharding.
        /// Calculates hash from string key and uses modulo operation.
        /// </summary>
        /// <param name="world">The world identifier (must be > 0).</param>
        /// <param name="key">The string key used for sharding calculation.</param>
        /// <returns>The shard Redis instance, or null if not found.</returns>
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

        /// <summary>
        /// Gets the shard size for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (must be > 0).</param>
        /// <returns>The count of Redis shards for the specified world.</returns>
        public int GetShardSize(uint world)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Unified has no shards.");

            return _worldShardRedis.TryGetValue(world, out var shardList) ? shardList.Count : 0;
        }

        /// <summary>
        /// Gets the list of world identifiers loaded from configuration (Redis:worlds).
        /// Use this to iterate all configured worlds without scanning Redis keys.
        /// </summary>
        /// <returns>Sorted list of configured world IDs.</returns>
        public IReadOnlyList<uint> GetConfiguredWorlds()
        {
            var worlds = new List<uint>(_worldGlobalRedis.Keys);
            worlds.Sort();
            return worlds;
        }
    }
}
