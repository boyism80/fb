using Google.FlatBuffers;
using Http.Util;
using RabbitMQ.Client;

namespace Http.Service
{
    /// <summary>
    /// Provides RabbitMQ messaging service functionality for publishing messages.
    /// Manages RabbitMQ connection and channel lifecycle for message publishing operations.
    /// </summary>
    public class RabbitMqService
    {
        private readonly IConnection _connection;
        private readonly IModel _channel;

        /// <summary>
        /// Initializes a new instance of the <see cref="RabbitMqService"/> class.
        /// Establishes connection to RabbitMQ server using configuration settings.
        /// </summary>
        /// <param name="configuration">The application configuration containing RabbitMQ connection settings.</param>
        public RabbitMqService(IConfiguration configuration)
        {
            var section = configuration.GetSection("RabbitMQ:Internal");
            var factory = new ConnectionFactory()
            {
                HostName = section.GetValue<string>("Host"),
                Port = section.GetValue<int>("Port"),
                UserName = section.GetValue<string>("Uid"),
                Password = section.GetValue<string>("Pwd")
            };


            _connection = factory.CreateConnection();
            _channel = _connection.CreateModel();
        }

        /// <summary>
        /// Publishes a FlatBuffer protocol message to the specified exchange with routing key.
        /// Converts the protocol to bytes and sends it through the RabbitMQ channel.
        /// </summary>
        /// <param name="protocol">The FlatBuffer protocol message to publish.</param>
        /// <param name="exchangeName">The name of the exchange to publish to.</param>
        /// <param name="routeKey">The routing key for message routing.</param>
        public void Publish(IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            _channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
        }
    }
}
