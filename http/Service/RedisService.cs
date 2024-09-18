using StackExchange.Redis;
using StackExchange.Redis.Extensions.Core.Configuration;

namespace http.Service
{
    public class RedisService
    {
        private readonly RedisConfiguration _configuration;

        public RedisService(IConfiguration configuration)
        {
            _configuration = configuration.GetSection("Redis").Get<RedisConfiguration>();
        }

        public IDatabase Connection
        {
            get
            {
                var redis = ConnectionMultiplexer.Connect(_configuration.ConfigurationOptions);
                return redis.GetDatabase(_configuration.Database);
            }
        }
    }
}
