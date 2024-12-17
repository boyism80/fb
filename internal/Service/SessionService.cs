using Http.Service;
using Http.Model.Redis;
using Http.Redis.Key;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    public class SessionService
    {
        private readonly RedisService _redisService;

        public SessionService(RedisService redisService)
        {
            _redisService = redisService;
        }

        public async Task<Session> Get(string name)
        {
            var key = new SessionKey().Key;
            var data = await _redisService.Connection.HashGetAsync(new RedisKey(key), new RedisValue(name));
            if (data.IsNull)
                return null;

            return JsonConvert.DeserializeObject<Session>(data.ToString());
        }
    }
}
