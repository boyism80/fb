using Http.Model.Redis;
using Http.Redis;
using Http.Redis.Key;
using Newtonsoft.Json;
using StackExchange.Redis;
using Response = fb.protocol._internal.response;

namespace Http.Service
{
    public class SessionService
    {
        private readonly RedisService _redisService;
        private readonly RabbitMqService _rabbitMqService;
        private const int SessionTtlSeconds = 300; // 5 minutes
        private const int MinTtlSeconds = 240; // 4 minutes (minimum TTL before refresh)

        private static readonly string GetAndDeleteSessionScript = """
            local session = redis.call('hget', @key, @name)
            if session ~= false then
                redis.call('hdel', @key, @name)
                return { 1, session }
            else
                return { 0 }
            end
            """;

        private static readonly string LoginScript = """
            local session = redis.call('hget', @key, @name)
            if session ~= false then
                redis.call('hdel', @key, @name)
                return { 0, session }
            else
                redis.call('hset', @key, @name, @session)
                redis.call('expire', @key, @ttl)
                return { 1 }
            end
            """;

        private static readonly string TryLoginScript = """
            local session = redis.call('hget', @key, @name)
            if session ~= false then
                return { 0, session }
            else
                redis.call('hset', @key, @name, @session)
                redis.call('expire', @key, @ttl)
                return { 1 }
            end
            """;

        private static readonly string SessionTtlRefreshScript = """
            local key = KEYS[1]
            local new_ttl = tonumber(ARGV[1])
            local min_ttl = tonumber(ARGV[2])

            local current_ttl = redis.call('ttl', key)
            if current_ttl < min_ttl then
                redis.call('expire', key, new_ttl)
                return 1
            else
                return 0
            end
            """;

        public SessionService(RedisService redisService, RabbitMqService rabbitMqService)
        {
            _redisService = redisService;
            _rabbitMqService = rabbitMqService;
        }

        public async Task<Session> Get(uint world, string name)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return null;
            var conn = redis.Connection;
            var data = await conn.HashGetAsync(new RedisKey(key), new RedisValue(name));
            if (data.IsNull)
                return null;

            return JsonConvert.DeserializeObject<Session>(data.ToString());
        }

        public async Task Set(uint world, string name, Session session)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return;
            var conn = redis.Connection;

            await conn.JsonHashSetAsync(new RedisKey(key), new RedisValue(name), session);
            await RefreshTTL(world);
        }

        public async Task Delete(uint world, string name)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return;
            var conn = redis.Connection;

            await conn.HashDeleteAsync(new RedisKey(key), name);
            await RefreshTTL(world);
        }

        public async Task<Session> GetAndDelete(uint world, string name)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return null;

            var redisResult = await redis.EvalAsync(GetAndDeleteSessionScript, new
            {
                key = new RedisKey(key),
                name = name
            });

            var found = (bool)redisResult[0];
            if (!found)
                return null;

            var sessionJson = redisResult[1].ToString();
            return JsonConvert.DeserializeObject<Session>(sessionJson);
        }

        public async Task<bool> Login(uint world, string name, Session session, bool force = false)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return false;

            var redisResult = await redis.EvalAsync(
                force ? LoginScript : TryLoginScript,
                new
                {
                    key = new RedisKey(key),
                    name = name,
                    session = JsonConvert.SerializeObject(session),
                    ttl = SessionTtlSeconds
                });

            var success = (bool)redisResult[0];
            if (success)
                return true;

            // If force is true and login failed, publish KickOut message
            if (force)
            {
                var existingSession = JsonConvert.DeserializeObject<Session>(redisResult[1].ToString());

                // Publish KickOut message to notify the game server to disconnect the existing user
                await _rabbitMqService.PublishAsync(new Response.KickOut
                {
                    Uid = existingSession.Uid,
                    Name = name
                }, AmqpRoute.Exchange, AmqpRoute.Unicast(existingSession));
            }

            return false;
        }

        public async Task RefreshTTL(uint world)
        {
            try
            {
                var key = new SessionKey().Key;
                var redis = _redisService.GetGlobalConnection(world);
                if (redis == null)
                    return;
                await redis.EvalAsync(
                    SessionTtlRefreshScript,
                    [new RedisKey(key)],
                    [(RedisValue)SessionTtlSeconds, (RedisValue)MinTtlSeconds]);
            }
            catch
            {
                // Silently ignore errors during TTL refresh
                // This prevents TTL refresh failures from affecting other operations
            }
        }

        public async Task<List<Session>> GetAllSessions(uint world)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return new List<Session>();

            var allSessions = await redis.Connection.HashGetAllAsync(new RedisKey(key));
            var sessions = new List<Session>();

            foreach (var entry in allSessions)
            {
                try
                {
                    var session = JsonConvert.DeserializeObject<Session>(entry.Value.ToString());
                    if (session != null)
                    {
                        sessions.Add(session);
                    }
                }
                catch
                {
                    // Skip invalid session entries
                }
            }

            return sessions;
        }
    }
}