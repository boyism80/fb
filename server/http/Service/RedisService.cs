using Http.Model;
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
        private readonly Dictionary<string, Redis> _redisBySection = new Dictionary<string, Redis>();

        /// <summary>
        /// Initializes a new instance of the <see cref="RedisService"/> class.
        /// Creates Redis connections for all configured instances using nested structure: Redis:{world}:{id}.
        /// Supports both integer world keys (e.g., "1", "2") and "unified-global" string key.
        /// </summary>
        /// <param name="configuration">The application configuration containing Redis connection settings.</param>
        public RedisService(IConfiguration configuration)
        {
            var redisSection = configuration.GetSection("Redis");

            // Parse nested structure: Redis:{world}:{id}
            // World keys can be integers (e.g., "1", "2") or "unified-global"
            foreach (var sectionChild in redisSection.GetChildren())
            {
                var worldKey = sectionChild.Key;
                foreach (var idChild in sectionChild.GetChildren())
                {
                    var id = idChild.Key;
                    var host = idChild.Get<RedisHost>();
                    if (host != null)
                    {
                        var redis = new Redis(host);
                        var key = $"{worldKey}:{id}";
                        _redisBySection.Add(key, redis);
                    }
                }
            }
        }

        /// <summary>
        /// Gets the Redis instance for the specified world and database index.
        /// Uses nested configuration structure: Redis:{world}:{db}
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="db">The database index within the world (-1, 0, 1, 2, etc.). Defaults to -1 (world-global Redis).</param>
        /// <returns>The Redis instance for the specified world and database, or null if not found.</returns>
        public Redis Redis(uint world, int db)
        {
            var worldKey = world == 0 ? "unified-global" : world.ToString();
            var key = $"{worldKey}:{db}";
            if (_redisBySection.ContainsKey(key) == false)
                return null;

            return _redisBySection[key];
        }

        /// <summary>
        /// Gets the shard size for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>The count of Redis shards excluding the default (-1) instance for the specified world.</returns>
        public int GetShardSize(uint world)
        {
            var worldKey = world == 0 ? "unified-global" : world.ToString();
            var sectionKey = $"{worldKey}:";
            return _redisBySection.Keys
                .Where(k => k.StartsWith(sectionKey) && k != $"{sectionKey}-1")
                .Count();
        }

        /// <summary>
        /// Gets the Redis instance for the specified world and ID using sharding logic.
        /// Uses modulo operation to distribute connections across available shards within the world.
        /// If id is null, uses world-global Redis (-1).
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="id">The unsigned integer ID to determine the target shard within the world. Null for world-global Redis.</param>
        /// <returns>The Redis instance for the calculated shard.</returns>
        public Redis Redis(uint world, uint? id = null)
        {
            if (id == null)
                return Redis(world, -1);
            else
            {
                var shardSize = GetShardSize(world);

                if (shardSize == 0)
                    return null;

                return Redis(world, (int)(id.Value % (uint)shardSize));
            }
        }

        public Redis Redis(uint world, string key)
        {
            ulong hash = 0;
            foreach (var b in Encoding.UTF8.GetBytes(key))
            {
                hash = hash * 31 + b;
            }
            return Redis(world, (int)(hash % (ulong)GetShardSize(world)));
        }
    }
}
