using Http.Redis;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    /// <summary>
    /// Represents an entry in the background commit queue for write-back operations.
    /// Contains SQL statement, Redis key, and hash information for deferred database writes.
    /// </summary>
    public class BackgroundCommitEntry
    {
        /// <summary>
        /// Gets or sets the SQL statement to be executed in the database.
        /// </summary>
        /// <value>The SQL command string for the database operation.</value>
        public required string SQL { get; set; }

        /// <summary>
        /// Gets or sets the Redis key associated with this database operation.
        /// </summary>
        /// <value>The Redis key string for cache invalidation or reference tracking.</value>
        public required string RedisKey { get; set; }

        /// <summary>
        /// Gets or sets the hash value used for database sharding.
        /// </summary>
        /// <value>The hash value that determines which database shard to use.</value>
        public required uint? Hash { get; set; }
    };

    /// <summary>
    /// Provides write-back functionality for deferred database operations.
    /// Queues database operations in Redis for asynchronous processing to improve performance.
    /// </summary>
    public class WriteBackService
    {
        private readonly RedisService _redisService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<WriteBackService> _logger;
        private static readonly TimeSpan _delay = TimeSpan.FromMilliseconds(500);

        /// <summary>
        /// Initializes a new instance of the <see cref="WriteBackService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for queue operations.</param>
        /// <param name="configuration">The application configuration.</param>
        /// <param name="serviceProvider">The service provider for dependency injection.</param>
        /// <param name="logger">The logger for recording write-back operations and errors.</param>
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

        /// <summary>
        /// Posts a database operation to the write-back queue for a specific database shard.
        /// The operation will be processed asynchronously by background workers.
        /// </summary>
        /// <param name="db">The database shard identifier.</param>
        /// <param name="sql">The SQL statement to execute.</param>
        /// <param name="key">The Redis key associated with this operation.</param>
        /// <param name="hash">The hash value for sharding.</param>
        /// <returns>A task representing the asynchronous queue operation.</returns>
        public async Task Post(string section, int db, string sql, string key, uint? hash)
        {
            var bufferKey = $"{Const.RedisBufferKey}:{db}";
            var redis = _redisService.Redis(section, bufferKey).Connection;
            await redis.ListRightPushAsync(
                new RedisKey(bufferKey),
                new RedisValue(JsonConvert.SerializeObject(new BackgroundCommitEntry
                {
                    SQL = sql,
                    RedisKey = key,
                    Hash = hash
                })));
        }

        /// <summary>
        /// Posts a database operation to the write-back queue using hash-based sharding.
        /// Automatically determines the appropriate database shard based on the hash value.
        /// </summary>
        /// <param name="hash">The hash value used for determining the database shard.</param>
        /// <param name="sql">The SQL statement to execute.</param>
        /// <param name="key">The Redis key associated with this operation.</param>
        /// <returns>A task representing the asynchronous queue operation.</returns>
        public async Task Post(string section, uint? hash, string sql, string key)
        {
            var sharedSize = _dbContext.GetShardDbSize(section);
            int db = hash != null ? (int)(hash % sharedSize) : -1;
            await Post(section, db, sql, key, hash);
        }
    }
}
