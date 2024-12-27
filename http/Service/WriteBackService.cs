using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    public class BackgroundCommitEntry
    {
        public required string SQL { get; set; }
        public required string RedisKey { get; set; }
    };

    public class WriteBackService
    {
        private readonly RedisService _redisService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<WriteBackService> _logger;
        private const string RedisBufferKey = "exec-buffer";
        private static readonly TimeSpan _delay = TimeSpan.FromMilliseconds(500);

        public WriteBackService(RedisService redisService,
            IConfiguration configuration,
            IServiceProvider serviceProvider,
            ILogger<WriteBackService> logger)
        {
            _redisService = redisService;
            _configuration = configuration;
            _logger = logger;
            _dbContext = ActivatorUtilities.CreateInstance<DbContext>(serviceProvider);
        }

        public async Task Post(int db, string sql, string key)
        {
            var bufferKey = $"{RedisBufferKey}:{db}";
            var redis = _redisService.Connection;
            await redis.ListRightPushAsync(
                new RedisKey(bufferKey),
                new RedisValue(JsonConvert.SerializeObject(new BackgroundCommitEntry
                {
                    SQL = sql,
                    RedisKey = key
                })));
        }

        public async Task Post(uint modKey, string sql, string key)
        {
            var db = modKey % _dbContext.SharedDbSize;
            await Post((int)db, sql, key);
        }
    }
}
