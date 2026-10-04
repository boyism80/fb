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

        public static readonly IDictionary<string, object> QueueArguments = new Dictionary<string, object> { ["x-queue-type"] = "quorum" };

        private readonly RabbitMqService _rabbitMqService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<WriteBackService> _logger;
        private readonly ConcurrentDictionary<string, byte> _declaredQueues = new ConcurrentDictionary<string, byte>();
        private readonly SemaphoreSlim _declareLock = new(1, 1);

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

        public async Task Post(uint world, int db, string sql, string key, uint? hash, CancellationToken cancellationToken = default)
        {
            var entry = new BackgroundCommitEntry
            {
                SQL = sql,
                RedisKey = key,
                Hash = hash
            };

            try
            {
                var queueName = GetWriteBackQueueName(world, db);
                await EnsureExchangeReadyAsync(queueName, cancellationToken);

                var json = JsonConvert.SerializeObject(entry);
                var body = Encoding.UTF8.GetBytes(json);
                await _rabbitMqService.PublishAsync(WriteBackExchangeName, queueName, body, persistent: true, cancellationToken);
            }
            catch (Exception ex)
            {
                _declaredQueues.TryRemove(GetWriteBackQueueName(world, db), out _);

                // Redis is already written and the cref stays incremented, so the Redis value remains
                // pinned until the write_back_failure row is replayed. Callers cannot roll back Redis.
                _logger.LogError(ex, "Failed to publish write-back message for world {World} db {Db} key {Key}: {Sql}", world, db, key, sql);
                try
                {
                    await using var logConn = _dbContext.GetGlobalConnection(world);
                    await WriteBackFailureRecorder.RecordAsync(logConn, world, db, entry, ex, CancellationToken.None);
                }
                catch (Exception logEx)
                {
                    _logger.LogCritical(logEx,
                        "Failed to insert write_back_failure for world {World} db {Db} key {Key}. Unrecorded SQL: {Sql}",
                        world,
                        db,
                        key,
                        sql);
                }
            }
        }

        private async Task EnsureExchangeReadyAsync(string queueName, CancellationToken cancellationToken)
        {
            if (_declaredQueues.ContainsKey(queueName))
            {
                return;
            }

            await _declareLock.WaitAsync(cancellationToken);
            try
            {
                if (_declaredQueues.ContainsKey(queueName))
                {
                    return;
                }

                await _rabbitMqService.WithChannelAsync(async channel =>
                {
                    await channel.ExchangeDeclareAsync(WriteBackExchangeName, ExchangeType.Direct, durable: true, autoDelete: false, arguments: null, passive: false, noWait: false, cancellationToken);
                    await channel.QueueDeclareAsync(queue: queueName, durable: true, exclusive: false, autoDelete: false, arguments: QueueArguments, passive: false, noWait: false, cancellationToken);
                    await channel.QueueBindAsync(queueName, WriteBackExchangeName, queueName, arguments: null, noWait: false, cancellationToken);
                }, cancellationToken);

                _declaredQueues.TryAdd(queueName, 0);
            }
            finally
            {
                _declareLock.Release();
            }
        }

        public async Task Post(uint world, uint? hash, string sql, string key, CancellationToken cancellationToken = default)
        {
            var sharedSize = _dbContext.GetShardDbSize(world);
            int db = hash != null ? (int)(hash % sharedSize) : -1;
            await Post(world, db, sql, key, hash, cancellationToken);
        }
    }
}
