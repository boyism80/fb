using Google.FlatBuffers;
using Http.Util;
using RabbitMQ.Client;

namespace Http.Service
{
    /// <summary>
    /// Provides RabbitMQ messaging service functionality for publishing messages.
    /// Manages a single unified-global RabbitMQ connection.
    /// </summary>
    public class RabbitMqService
    {
        private readonly IConfiguration _configuration;
        private readonly Lazy<IConnection> _connection;
        private readonly Lazy<IModel> _channel;
        private readonly object _writeBackLock = new object();

        /// <summary>
        /// Initializes a new instance of the <see cref="RabbitMqService"/> class.
        /// </summary>
        /// <param name="configuration">The application configuration containing RabbitMQ connection settings.</param>
        public RabbitMqService(IConfiguration configuration)
        {
            _configuration = configuration;
            _connection = new Lazy<IConnection>(GetConnection);
            _channel = new Lazy<IModel>(GetChannel);
        }

        /// <summary>
        /// Gets or creates a RabbitMQ connection for unified-global RabbitMQ.
        /// </summary>
        /// <returns>The RabbitMQ connection for unified-global.</returns>
        private IConnection GetConnection()
        {
            var rabbitMqSection = _configuration.GetSection("RabbitMQ:Internal");
            if (rabbitMqSection == null || !rabbitMqSection.Exists())
            {
                throw new Exception("RabbitMQ configuration not found for unified-global");
            }

            var factory = new ConnectionFactory()
            {
                HostName = rabbitMqSection.GetValue<string>("Host"),
                Port = rabbitMqSection.GetValue<int>("Port"),
                UserName = rabbitMqSection.GetValue<string>("Uid"),
                Password = rabbitMqSection.GetValue<string>("Pwd")
            };

            return factory.CreateConnection();
        }

        /// <summary>
        /// Gets or creates a RabbitMQ channel for unified-global RabbitMQ.
        /// </summary>
        /// <returns>The RabbitMQ channel for unified-global.</returns>
        private IModel GetChannel()
        {
            return _connection.Value.CreateModel();
        }

        /// <summary>
        /// Publishes a FlatBuffer protocol message to the specified exchange with routing key.
        /// Uses unified-global RabbitMQ connection. The routing key must be explicitly specified.
        /// </summary>
        /// <param name="protocol">The FlatBuffer protocol message to publish.</param>
        /// <param name="exchangeName">The name of the exchange to publish to.</param>
        /// <param name="routeKey">The routing key for message routing (must include world prefix if needed).</param>
        public void Publish(IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            var channel = _channel.Value;
            channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
        }

        /// <summary>
        /// Name of the write-back exchange (direct, durable).
        /// </summary>
        public const string WriteBackExchangeName = "write-back";

        /// <summary>
        /// Builds the write-back queue name for the given world and database shard.
        /// </summary>
        /// <param name="world">The world identifier.</param>
        /// <param name="db">The database shard (-1 for global, 0-based for data).</param>
        /// <returns>Queue name used for both binding and routing key.</returns>
        public static string GetWriteBackQueueName(uint world, int db)
        {
            return $"{WriteBackExchangeName}.{world}.{db}";
        }

        /// <summary>
        /// Publishes a write-back entry to the RabbitMQ queue for the given world and database shard.
        /// Declares the write-back exchange and queue if they do not exist (idempotent).
        /// Uses RabbitMQ:Internal connection.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2).</param>
        /// <param name="db">The database shard identifier (-1 for global, 0-based for data).</param>
        /// <param name="body">The message body (typically JSON-serialized BackgroundCommitEntry).</param>
        public void PublishWriteBack(uint world, int db, byte[] body)
        {
            if (body == null || body.Length == 0)
                return;

            lock (_writeBackLock)
            {
                var channel = _channel.Value;
                channel.ExchangeDeclare(WriteBackExchangeName, ExchangeType.Direct, durable: true);
                var queueName = GetWriteBackQueueName(world, db);
                channel.QueueDeclare(queue: queueName, durable: true, exclusive: false, autoDelete: false, arguments: null);
                channel.QueueBind(queueName, WriteBackExchangeName, queueName);
                var props = channel.CreateBasicProperties();
                props.Persistent = true;
                channel.BasicPublish(WriteBackExchangeName, queueName, props, body);
            }
        }
    }
}
