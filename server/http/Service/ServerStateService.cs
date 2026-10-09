using Http.Redis;
using Http.Redis.Key;
using Newtonsoft.Json;
using StackExchange.Redis;
using Protocol = fb.protocol._internal;

namespace Http.Service
{
    public class ServerStateService
    {
        private static readonly TimeSpan HeartbeatTtl = TimeSpan.FromSeconds(5);

        private readonly RedisService _redisService;

        public ServerStateService(RedisService redisService)
        {
            _redisService = redisService;
        }

        public class ServerInfo
        {
            public uint? World { get; set; }

            public string Service { get; set; } = string.Empty;

            public byte Id { get; set; }

            public string Name { get; set; } = string.Empty;

            public string IP { get; set; } = string.Empty;

            public ushort Port { get; set; }

            public uint Online { get; set; }
        }

        public async Task<List<ServerInfo>> GetRunningServers()
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return new List<ServerInfo>();

            var expired = DateTimeOffset.UtcNow.Subtract(HeartbeatTtl).ToUnixTimeMilliseconds();
            await redis.Connection.SortedSetRemoveRangeByScoreAsync(HeartBeatKey.Index, double.NegativeInfinity, expired, Exclude.Stop);

            var members = await redis.Connection.SortedSetRangeByScoreAsync(HeartBeatKey.Index, expired, double.PositiveInfinity);
            if (members.Length == 0)
                return new List<ServerInfo>();

            var keys = members.Select(member => new RedisKey(HeartBeatKey.Prefix + member)).ToArray();
            var values = await redis.Connection.StringGetAsync(keys);
            var servers = new List<ServerInfo>();

            for (var i = 0; i < members.Length; i++)
            {
                var value = values[i];
                if (value.IsNull)
                    continue;

                var parts = members[i].ToString().Split(':');
                if (parts.Length != 3)
                    continue;

                uint? world;
                if (parts[0] == "cross")
                {
                    world = null;
                }
                else if (uint.TryParse(parts[0], out var parsed))
                {
                    world = parsed;
                }
                else
                {
                    continue;
                }

                var service = parts[1];
                if (!byte.TryParse(parts[2], out var id))
                    continue;

                try
                {
                    var config = JsonConvert.DeserializeObject<HostConfig>(value.ToString());
                    if (config != null)
                    {
                        servers.Add(new ServerInfo
                        {
                            World = world,
                            Service = service,
                            Id = id,
                            Name = config.Name,
                            IP = config.IP,
                            Port = config.Port,
                            Online = config.Online
                        });
                    }
                }
                catch
                {
                    // Skip invalid JSON
                }
            }

            return servers
                .OrderBy(s => s.World.HasValue)
                .ThenBy(s => s.World)
                .ThenBy(s => s.Service)
                .ThenBy(s => s.Id)
                .ToList();
        }

        public async Task<bool> HasRunningServers()
        {
            var servers = await GetRunningServers();
            return servers.Count > 0;
        }

        public async Task<bool> UpdateHeartbeat(uint? world, Protocol.Service service, byte id, string name, string ip, ushort port, uint online)
        {
            try
            {
                var redis = _redisService.GetUnifiedConnection();
                if (redis == null)
                    return false;

                var config = new HostConfig
                {
                    Name = name,
                    IP = ip,
                    Port = port,
                    Online = online
                };
                var key = new HeartBeatKey { World = world, Service = service, Id = id };
                var json = JsonConvert.SerializeObject(config);

                var transaction = redis.Connection.CreateTransaction();
                _ = transaction.StringSetAsync(key.Key, json, HeartbeatTtl);
                _ = transaction.SortedSetAddAsync(HeartBeatKey.Index, key.Member, DateTimeOffset.UtcNow.ToUnixTimeMilliseconds());
                return await transaction.ExecuteAsync();
            }
            catch
            {
                return false;
            }
        }

        public async Task<HostConfig> GetHostConfig(uint? world, Protocol.Service service, byte id)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return null;

            var key = new HeartBeatKey { World = world, Service = service, Id = id };
            return await redis.Connection.JsonGetAsync<HostConfig>(key.Key);
        }

        public async Task<List<ServerInfo>> ListLiveCrossServers()
        {
            var servers = await GetRunningServers();
            return servers
                .Where(s => !s.World.HasValue && s.Service == Protocol.Service.Game.ToString())
                .ToList();
        }

        public class HostConfig
        {
            [JsonProperty("Name")]
            public string Name { get; set; } = string.Empty;

            [JsonProperty("IP")]
            public string IP { get; set; } = string.Empty;

            [JsonProperty("Port")]
            public ushort Port { get; set; }

            [JsonProperty("Online")]
            public uint Online { get; set; }
        }
    }
}
