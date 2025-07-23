using Http.Model.Redis;
using Http.Redis.Key;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    /// <summary>
    /// Provides session management functionality for user sessions stored in Redis.
    /// Handles retrieval of user session information including host server assignments.
    /// </summary>
    public class SessionService
    {
        private readonly RedisService _redisService;

        /// <summary>
        /// Initializes a new instance of the <see cref="SessionService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for accessing session data.</param>
        public SessionService(RedisService redisService)
        {
            _redisService = redisService;
        }

        /// <summary>
        /// Retrieves a user session by character name from Redis storage.
        /// </summary>
        /// <param name="name">The character name to look up the session for.</param>
        /// <returns>The session information if found; otherwise, null.</returns>
        public async Task<Session> Get(string name)
        {
            var key = new SessionKey().Key;
            var conn = _redisService.Redis(-1).Connection;
            var data = await conn.HashGetAsync(new RedisKey(key), new RedisValue(name));
            if (data.IsNull)
                return null;

            return JsonConvert.DeserializeObject<Session>(data.ToString());
        }
    }
}
