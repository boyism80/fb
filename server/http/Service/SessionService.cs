using Http.Model.Redis;
using Http.Redis;
using Http.Redis.Key;
using Newtonsoft.Json;
using StackExchange.Redis;
using Response = fb.protocol._internal.response;

namespace Http.Service
{
    /// <summary>
    /// Provides session management functionality for user sessions stored in Redis.
    /// Handles session creation, deletion, and TTL management.
    /// </summary>
    public class SessionService
    {
        private readonly RedisService _redisService;
        private readonly RabbitMqService _rabbitMqService;
        private const int SessionTtlSeconds = 300; // 5 minutes
        private const int MinTtlSeconds = 240; // 4 minutes (minimum TTL before refresh)

        /// <summary>
        /// Lua script for refreshing session TTL atomically.
        /// Only refreshes if the current TTL is below the minimum threshold.
        /// </summary>
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

        /// <summary>
        /// Initializes a new instance of the <see cref="SessionService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for accessing Redis connections.</param>
        /// <param name="rabbitMqService">The RabbitMQ service for publishing messages.</param>
        public SessionService(RedisService redisService, RabbitMqService rabbitMqService)
        {
            _redisService = redisService;
            _rabbitMqService = rabbitMqService;
        }

        /// <summary>
        /// Retrieves a session by name.
        /// </summary>
        /// <param name="name">The character name to retrieve the session for.</param>
        /// <returns>The session if found; otherwise, null.</returns>
        public async Task<Session> Get(string name)
        {
            var key = new SessionKey().Key;
            var conn = _redisService.Redis(-1).Connection;
            var data = await conn.HashGetAsync(new RedisKey(key), new RedisValue(name));
            if (data.IsNull)
                return null;

            return JsonConvert.DeserializeObject<Session>(data.ToString());
        }

        /// <summary>
        /// Sets a session for a character name and refreshes the TTL.
        /// </summary>
        /// <param name="name">The character name.</param>
        /// <param name="session">The session data to store.</param>
        public async Task Set(string name, Session session)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.Redis(-1);
            var conn = redis.Connection;

            await conn.JsonHashSetAsync(new RedisKey(key), new RedisValue(name), session);
            await RefreshTTL();
        }

        /// <summary>
        /// Deletes a session for a character name and refreshes the TTL.
        /// </summary>
        /// <param name="name">The character name to delete the session for.</param>
        public async Task Delete(string name)
        {
            var key = new SessionKey().Key;
            var conn = _redisService.Redis(-1).Connection;

            await conn.HashDeleteAsync(new RedisKey(key), name);
            await RefreshTTL();
        }

        /// <summary>
        /// Attempts to login by setting a session using the login.lua script.
        /// If a session already exists, it publishes a KickOut message.
        /// </summary>
        /// <param name="name">The character name.</param>
        /// <param name="session">The session data to store.</param>
        /// <returns>True if login was successful (new session created), false if an existing session was found.</returns>
        public async Task<bool> Login(string name, Session session)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.Redis(-1);

            var redisResult = await redis.ScriptEvaluateAsync("login.lua", new
            {
                key = new RedisKey(key),
                name = name,
                session = JsonConvert.SerializeObject(session),
                ttl = SessionTtlSeconds
            });

            var success = (bool)redisResult[0];
            if (success)
            {
                return true;
            }

            var existingSession = JsonConvert.DeserializeObject<Session>(redisResult[1].ToString());

            // Publish KickOut message to notify the game server to disconnect the existing user
            _rabbitMqService.Publish(new Response.KickOut
            {
                Uid = existingSession.Uid,
                Name = name
            }, "amq.direct", $"fb.game.{existingSession.Host}");

            return false;
        }

        /// <summary>
        /// Refreshes the TTL of the session hash key using an atomic Lua script.
        /// Only refreshes if the current TTL is below the minimum threshold to prevent redundant operations.
        /// </summary>
        public async Task RefreshTTL()
        {
            try
            {
                var key = new SessionKey().Key;
                var redis = _redisService.Redis(-1);
                var script = LuaScript.Prepare(SessionTtlRefreshScript).Load(redis.GetServer());

                await redis.Connection.ScriptEvaluateAsync(script.Hash,
                    keys: [new RedisKey(key)],
                    values: [(RedisValue)SessionTtlSeconds, (RedisValue)MinTtlSeconds]);
            }
            catch
            {
                // Silently ignore errors during TTL refresh
                // This prevents TTL refresh failures from affecting other operations
            }
        }
    }
}
