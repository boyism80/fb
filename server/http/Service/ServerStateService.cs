using Http.Redis;
using Http.Redis.Key;
using Newtonsoft.Json;
using Protocol = fb.protocol._internal;

namespace Http.Service
{
    /// <summary>
    /// Provides server state management functionality.
    /// Handles retrieval of running server information and user session management.
    /// </summary>
    public class ServerStateService
    {
        private readonly RedisService _redisService;

        /// <summary>
        /// Initializes a new instance of the <see cref="ServerStateService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for accessing server state data.</param>
        public ServerStateService(RedisService redisService)
        {
            _redisService = redisService;
        }

        /// <summary>
        /// Represents a running server with its configuration information.
        /// </summary>
        public class ServerInfo
        {
            /// <summary>
            /// Gets or sets the service type (Game, Login, etc.).
            /// </summary>
            public string Service { get; set; } = string.Empty;

            /// <summary>
            /// Gets or sets the server ID.
            /// </summary>
            public byte Id { get; set; }

            /// <summary>
            /// Gets or sets the server name.
            /// </summary>
            public string Name { get; set; } = string.Empty;

            /// <summary>
            /// Gets or sets the server IP address.
            /// </summary>
            public string IP { get; set; } = string.Empty;

            /// <summary>
            /// Gets or sets the server port.
            /// </summary>
            public ushort Port { get; set; }
        }

        /// <summary>
        /// Retrieves all currently running servers from Redis heart-beat keys.
        /// </summary>
        /// <returns>A list of running server information.</returns>
        public async Task<List<ServerInfo>> GetRunningServers()
        {
            var redis = _redisService.Redis(0);
            if (redis == null)
                return new List<ServerInfo>();

            var keys = await redis.Connection.ScanKeysAsync("heart-beat:*", 1000);
            var servers = new List<ServerInfo>();

            foreach (var key in keys)
            {
                var keyStr = key.ToString();
                // Parse key format: heart-beat:World:Service:Id
                var parts = keyStr.Split(':');
                if (parts.Length != 4)
                    continue;

                var section = parts[1];
                var service = parts[2];
                if (!byte.TryParse(parts[3], out var id))
                    continue;

                var value = await redis.Connection.StringGetAsync(key);
                if (value.IsNull)
                    continue;

                try
                {
                    var config = JsonConvert.DeserializeObject<HostConfig>(value.ToString());
                    if (config != null)
                    {
                        servers.Add(new ServerInfo
                        {
                            Service = service,
                            Id = id,
                            Name = config.Name,
                            IP = config.IP,
                            Port = config.Port
                        });
                    }
                }
                catch
                {
                    // Skip invalid JSON
                }
            }

            return servers.OrderBy(s => s.Service).ThenBy(s => s.Id).ToList();
        }

        /// <summary>
        /// Checks if any servers are currently running.
        /// </summary>
        /// <returns>True if any servers are running; otherwise, false.</returns>
        public async Task<bool> HasRunningServers()
        {
            var redis = _redisService.Redis(0);
            if (redis == null)
                return false;

            var keys = await redis.Connection.ScanKeysAsync("heart-beat:*", 1000);
            return keys.Count > 0;
        }

        /// <summary>
        /// Updates the heartbeat for a server.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="service">The service type (Game, Login, etc.).</param>
        /// <param name="id">The server ID.</param>
        /// <param name="name">The server name.</param>
        /// <param name="ip">The server IP address.</param>
        /// <param name="port">The server port.</param>
        /// <returns>True if the heartbeat was successfully updated; otherwise, false.</returns>
        public async Task<bool> UpdateHeartbeat(uint world, Protocol.Service service, byte id, string name, string ip, ushort port)
        {
            try
            {
                var redis = _redisService.Redis(0); // unified-global is 0
                if (redis == null)
                    return false;

                var config = new HostConfig
                {
                    Name = name,
                    IP = ip,
                    Port = port
                };
                var key = new HeartBeatKey { World = world, Service = service, Id = id };
                var json = JsonConvert.SerializeObject(config);

                await redis.Connection.StringSetAsync(key.Key, json);
                await redis.Connection.KeyExpireAsync(key.Key, TimeSpan.FromSeconds(5));

                return true;
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// Retrieves the host configuration for a specific server.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="service">The service type (Game, Login, etc.).</param>
        /// <param name="id">The server ID.</param>
        /// <returns>The host configuration if found; otherwise, null.</returns>
        public async Task<HostConfig> GetHostConfig(uint world, Protocol.Service service, byte id)
        {
            var redis = _redisService.Redis(0); // unified-global is 0
            if (redis == null)
                return null;

            var key = new HeartBeatKey { World = world, Service = service, Id = id };
            return await redis.Connection.JsonGetAsync<HostConfig>(key.Key);
        }

        /// <summary>
        /// Represents host configuration stored in heart-beat keys.
        /// </summary>
        public class HostConfig
        {
            [JsonProperty("Name")]
            public string Name { get; set; } = string.Empty;

            [JsonProperty("IP")]
            public string IP { get; set; } = string.Empty;

            [JsonProperty("Port")]
            public ushort Port { get; set; }
        }
    }
}

