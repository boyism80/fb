using Newtonsoft.Json;
using RabbitMQ.Client;
using System.Collections.Concurrent;
using System.Text;

namespace Http.Service
{
    public class BackgroundCommitEntry
    {
        public required string SQL { get; set; }
        public required string RedisKey { get; set; }
        public required uint? Hash { get; set; }
    };

    public class WriteBackService
    {
        public const string WriteBackExchangeName = "write-back";

        public static string GetWriteBackQueueName(uint world, int db) => $"{WriteBackExchangeName}.{world}.{db}";

        private readonly RabbitMqService _rabbitMqService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<WriteBackService> _logger;
        private readonly ConcurrentDictionary<string, byte> _declaredQueues = new ConcurrentDictionary<string, byte>();
        private readonly object _declareLock = new object();

        public WriteBackService(RabbitMqService rabbitMqService,
            IConfiguration configuration,
            IServiceProvider serviceProvider,
            ILogger<WriteBackService> logger)
        {
            _rabbitMqService = rabbitMqService;
            _configuration = configuration;
            _logger = logger;
            _dbContext = ActivatorUtilities.CreateInstance<DbContext>(serviceProvider);
        }

        public Task Post(uint world, int db, string sql, string key, uint? hash)
        {
            try
            {
                var queueName = GetWriteBackQueueName(world, db);
                EnsureExchangeReady(queueName);

                var json = JsonConvert.SerializeObject(new BackgroundCommitEntry
                {
                    SQL = sql,
                    RedisKey = key,
                    Hash = hash
                });
                var body = Encoding.UTF8.GetBytes(json);
                _rabbitMqService.Publish(WriteBackExchangeName, queueName, body, persistent: true);
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to publish write-back message for world {World} db {Db}", world, db);
            }

            return Task.CompletedTask;
        }

        private void EnsureExchangeReady(string queueName)
        {
            if (_declaredQueues.ContainsKey(queueName))
                return;

            lock (_declareLock)
            {
                if (_declaredQueues.ContainsKey(queueName))
                    return;

                _rabbitMqService.WithChannel(channel =>
                {
                    channel.ExchangeDeclare(WriteBackExchangeName, ExchangeType.Direct, durable: true);
                    channel.QueueDeclare(queue: queueName, durable: true, exclusive: false, autoDelete: false, arguments: null);
                    channel.QueueBind(queueName, WriteBackExchangeName, queueName);
                });
                _declaredQueues.TryAdd(queueName, 0);
            }
        }

        public async Task Post(uint world, uint? hash, string sql, string key)
        {
            var sharedSize = _dbContext.GetShardDbSize(world);
            int db = hash != null ? (int)(hash % sharedSize) : -1;
            await Post(world, db, sql, key, hash);
        }
    }
}
