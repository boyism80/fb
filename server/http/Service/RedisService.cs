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
        private readonly Dictionary<int, Redis> _redis = new Dictionary<int, Redis>();

        /// <summary>
        /// Gets the number of Redis shards available for distribution.
        /// </summary>
        /// <value>The total count of Redis shards excluding the default (-1) instance.</value>
        public int ShardSize { get; private set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="RedisService"/> class.
        /// Creates Redis connections for all configured instances and calculates shard size.
        /// </summary>
        /// <param name="configuration">The application configuration containing Redis connection settings.</param>
        public RedisService(IConfiguration configuration)
        {
            var size = 0;
            foreach (var section in configuration.GetSection("Redis").GetChildren())
            {
                var id = int.Parse(section.Key);
                var host = section.Get<RedisHost>();
                _redis.Add(id, new Redis(host));

                if (id != -1)
                    size++;
            }

            ShardSize = size;
        }

        /// <summary>
        /// Gets the Redis instance for the specified shard ID.
        /// </summary>
        /// <param name="id">The shard ID to retrieve the Redis instance for.</param>
        /// <returns>The Redis instance for the specified shard, or null if not found.</returns>
        public Redis Redis(int id)
        {
            if (_redis.ContainsKey(id) == false)
                return null;

            return _redis[id];
        }

        /// <summary>
        /// Gets the Redis instance for the specified unsigned integer ID using modulo sharding.
        /// </summary>
        /// <param name="id">The unsigned integer ID to determine the target shard.</param>
        /// <returns>The Redis instance for the calculated shard.</returns>
        public Redis Redis(uint id)
        {
            return Redis((int)(id % ShardSize));
        }

        public Redis Redis(uint? id)
        {
            if (id == null)
                return Redis(-1);
            else
                return Redis(id.Value);
        }

        /// <summary>
        /// Gets the Redis instance for the specified string key using hash-based sharding.
        /// Uses a simple hash algorithm to distribute keys across shards.
        /// </summary>
        /// <param name="key">The string key to determine the target shard.</param>
        /// <returns>The Redis instance for the calculated shard based on key hash.</returns>
        public Redis Redis(string key)
        {
            ulong hash = 0;
            foreach (var b in Encoding.UTF8.GetBytes(key))
            {
                hash = hash * 31 + b;
            }

            return Redis((int)(hash % (ulong)ShardSize));
        }

        /// <summary>
        /// Gets the Redis instance for the specified Redis value key using its hash method.
        /// </summary>
        /// <param name="key">The Redis value key implementing <see cref="IRedisValueKey"/>.</param>
        /// <returns>The Redis instance for the calculated shard based on key hash.</returns>
        public Redis Redis(IRedisValueKey key)
        {
            var hash = key.GetHash();
            if (hash == null)
                return Redis(-1);
            else
                return Redis(hash.Value);
        }

        /// <summary>
        /// Gets the Redis instance for the specified Redis key using string-based sharding.
        /// </summary>
        /// <param name="key">The Redis key to determine the target shard.</param>
        /// <returns>The Redis instance for the calculated shard based on key string representation.</returns>
        public Redis Redis(RedisKey key)
        {
            return Redis(key.ToString());
        }
    }
}
