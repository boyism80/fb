using Http.Redis;
using Http.Redis.Key;
using Newtonsoft.Json;
using Protocol = fb.protocol._internal;

namespace Http.Service
{
    public class ServerStateService
    {
        private readonly RedisService _redisService;

        public ServerStateService(RedisService redisService)
        {
            _redisService = redisService;
        }

        public class ServerInfo
        {
            public uint World { get; set; }

            public string Service { get; set; } = string.Empty;

            public byte Id { get; set; }

            public string Name { get; set; } = string.Empty;

            public string IP { get; set; } = string.Empty;

            public ushort Port { get; set; }
        }

        public async Task<List<ServerInfo>> GetRunningServers()
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return new List<ServerInfo>();

            var keys = await redis.Connection.ScanKeysAsync("fb:heart-beat:*", 1000);
            var servers = new List<ServerInfo>();

            foreach (var key in keys)
            {
                var keyStr = key.ToString();
                // Parse key format: fb:heart-beat:World:Service:Id
                var parts = keyStr.Split(':');
                if (parts.Length != 5)
                    continue;

                if (!uint.TryParse(parts[2], out var world))
                    continue;

                var service = parts[3];
                if (!byte.TryParse(parts[4], out var id))
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
                            World   = world,
                            Service = service,
                            Id      = id,
                            Name    = config.Name,
                            IP      = config.IP,
                            Port    = config.Port
                        });
                    }
                }
                catch
                {
                    // Skip invalid JSON
                }
            }

            return servers
                .OrderBy(s => s.World)
                .ThenBy(s => s.Service)
                .ThenBy(s => s.Id)
                .ToList();
        }

        public async Task<bool> HasRunningServers()
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return false;

            var keys = await redis.Connection.ScanKeysAsync("fb:heart-beat:*", 1000);
            return keys.Count > 0;
        }

        public async Task<bool> UpdateHeartbeat(uint world, Protocol.Service service, byte id, string name, string ip, ushort port)
        {
            try
            {
                var redis = _redisService.GetUnifiedConnection(); // unified is 0
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

        public async Task<HostConfig> GetHostConfig(uint world, Protocol.Service service, byte id)
        {
            var redis = _redisService.GetUnifiedConnection(); // unified is 0
            if (redis == null)
                return null;

            var key = new HeartBeatKey { World = world, Service = service, Id = id };
            return await redis.Connection.JsonGetAsync<HostConfig>(key.Key);
        }

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
