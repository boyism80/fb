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

            public string Role { get; set; } = "home";

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
                var parts = keyStr.Split(':');
                if (parts.Length != 5)
                    continue;

                var role = parts[2] == "cross" ? "cross" : "home";
                uint world = 0;
                if (role == "home" && !uint.TryParse(parts[2], out world))
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
                            World = world,
                            Role = role,
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

            return servers
                .OrderBy(s => s.Role)
                .ThenBy(s => s.World)
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

        public async Task<bool> UpdateHeartbeat(uint world, Protocol.Service service, byte id, string name, string ip, ushort port, Protocol.ProcessRole role = Protocol.ProcessRole.Home)
        {
            try
            {
                var redis = _redisService.GetUnifiedConnection();
                if (redis == null)
                    return false;

                var roleName = AmqpRoute.Name(role);
                var config = new HostConfig
                {
                    Name = name,
                    IP = ip,
                    Port = port,
                    Role = roleName
                };
                var key = new HeartBeatKey { Role = role, World = world, Service = service, Id = id };
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
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return null;

            var key = new HeartBeatKey { Role = Protocol.ProcessRole.Home, World = world, Service = service, Id = id };
            return await redis.Connection.JsonGetAsync<HostConfig>(key.Key);
        }

        public async Task<HostConfig> GetCrossHost(byte id)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return null;

            var key = new HeartBeatKey { Role = Protocol.ProcessRole.Cross, Service = Protocol.Service.Game, Id = id };
            return await redis.Connection.JsonGetAsync<HostConfig>(key.Key);
        }

        public async Task<List<ServerInfo>> ListLiveCrossServers()
        {
            var servers = await GetRunningServers();
            return servers
                .Where(s => s.Role == "cross" && s.Service == Protocol.Service.Game.ToString())
                .ToList();
        }

        public async Task<ServerInfo> PickLiveCrossServer(string matchId)
        {
            var ordered = (await ListLiveCrossServers())
                .OrderBy(s => s.Id)
                .ToList();
            if (ordered.Count == 0)
                return null;

            if (string.IsNullOrEmpty(matchId))
                return ordered[0];

            ulong hash = 0;
            foreach (var ch in matchId)
            {
                hash = hash * 31 + ch;
            }
            return ordered[(int)(hash % (ulong)ordered.Count)];
        }

        public class HostConfig
        {
            [JsonProperty("Name")]
            public string Name { get; set; } = string.Empty;

            [JsonProperty("IP")]
            public string IP { get; set; } = string.Empty;

            [JsonProperty("Port")]
            public ushort Port { get; set; }

            [JsonProperty("Role")]
            public string Role { get; set; } = "home";
        }
    }
}
