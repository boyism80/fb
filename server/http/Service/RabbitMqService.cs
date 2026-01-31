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
    }
}
