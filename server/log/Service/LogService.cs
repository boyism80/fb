using System.Text.Json;
using Http.Service;
using StackExchange.Redis;

namespace Log.Service
{
    /// <summary>
    /// Service for storing log entries in Redis with performance-optimized sharding.
    /// Distributes log entries across Redis shards using random selection for load balancing.
    /// </summary>
    public class LogService
    {
        private readonly RedisService _redisService;
        private readonly ILogger<LogService> _logger;
        private readonly Random _random = new Random();
        private const string LogListKey = "log:pending";

        /// <summary>
        /// Initializes a new instance of the <see cref="LogService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for sharding.</param>
        /// <param name="logger">The logger instance.</param>
        public LogService(RedisService redisService, ILogger<LogService> logger)
        {
            _redisService = redisService;
            _logger = logger;
        }

        /// <summary>
        /// Stores log entries in Redis using random sharding for performance.
        /// Distributes logs randomly across all available Redis shards.
        /// </summary>
        /// <param name="logs">The array of JSON log entries to store.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        public async Task StoreLogsAsync(JsonElement[] logs)
        {
            if (logs == null || logs.Length == 0)
                return;

            if (_redisService.ShardSize == 0)
            {
                _logger.LogWarning("No Redis shards available");
                return;
            }

            // Serialize log entries to JSON strings
            var logStrings = logs.Select(log => log.GetRawText()).ToArray();

            // Get Redis instance using random selection
            var shardIndex = _random.Next(_redisService.ShardSize);
            var redis = _redisService.Redis(shardIndex);
            if (redis == null)
            {
                _logger.LogWarning($"Redis instance not available for shard {shardIndex}");
                return;
            }

            // Push logs to Redis list
            await redis.Connection.ListRightPushAsync(LogListKey, logStrings.Select(s => (RedisValue)s).ToArray());
        }
    }
}
