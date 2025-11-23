using Http.Redis;
using Http.Redis.Key;
using Newtonsoft.Json;
using StackExchange.Redis;

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
            var redis = _redisService.Redis(-1);
            var keys = await redis.Connection.ScanKeysAsync("heart-beat:*", 1000);
            var servers = new List<ServerInfo>();

            foreach (var key in keys)
            {
                var keyStr = key.ToString();
                // Parse key format: heart-beat:Service:Id
                var parts = keyStr.Split(':');
                if (parts.Length != 3)
                    continue;

                var service = parts[1];
                if (!byte.TryParse(parts[2], out var id))
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
            var redis = _redisService.Redis(-1);
            var keys = await redis.Connection.ScanKeysAsync("heart-beat:*", 1000);
            return keys.Count > 0;
        }

        /// <summary>
        /// Represents host configuration stored in heart-beat keys.
        /// </summary>
        private class HostConfig
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

