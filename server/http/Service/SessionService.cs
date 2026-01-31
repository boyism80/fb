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
        /// Retrieves a session by world and name.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="name">The character name to retrieve the session for.</param>
        /// <returns>The session if found; otherwise, null.</returns>
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

        /// <summary>
        /// Sets a session for a character name in the specified world and refreshes the TTL.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="name">The character name.</param>
        /// <param name="session">The session data to store.</param>
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

        /// <summary>
        /// Deletes a session for a character name in the specified world and refreshes the TTL.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="name">The character name to delete the session for.</param>
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

        /// <summary>
        /// Atomically gets and deletes a session by world and name using the get_and_delete_session.lua script.
        /// Returns the session if it existed, null otherwise.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="name">The character name to get and delete the session for.</param>
        /// <returns>The session if it existed and was deleted; otherwise, null.</returns>
        public async Task<Session> GetAndDelete(uint world, string name)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return null;

            var redisResult = await redis.ScriptEvaluateAsync("get_and_delete_session.lua", new
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

        /// <summary>
        /// Attempts to login by setting a session in the specified world.
        /// If force is false, uses try_login.lua script which does not delete existing sessions.
        /// If force is true, uses login.lua script which deletes existing sessions and publishes KickOut message.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="name">The character name.</param>
        /// <param name="session">The session data to store.</param>
        /// <param name="force">If true, replaces existing session. If false, fails if session exists.</param>
        /// <returns>True if login was successful (new session created), false if an existing session was found.</returns>
        public async Task<bool> Login(uint world, string name, Session session, bool force = false)
        {
            var key = new SessionKey().Key;
            var redis = _redisService.GetGlobalConnection(world);
            if (redis == null)
                return false;

            string scriptName = force ? "login.lua" : "try_login.lua";
            var redisResult = await redis.ScriptEvaluateAsync(scriptName, new
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
                _rabbitMqService.Publish(new Response.KickOut
                {
                    Uid = existingSession.Uid,
                    Name = name
                }, "amq.direct", $"fb.{world}.game.{existingSession.Host}");
            }

            return false;
        }

        /// <summary>
        /// Refreshes the TTL of the session hash key in the specified world using an atomic Lua script.
        /// Only refreshes if the current TTL is below the minimum threshold to prevent redundant operations.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        public async Task RefreshTTL(uint world)
        {
            try
            {
                var key = new SessionKey().Key;
                var redis = _redisService.GetGlobalConnection(world);
                if (redis == null)
                    return;
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

        /// <summary>
        /// Gets all active sessions for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>A list of all active sessions for the world.</returns>
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
