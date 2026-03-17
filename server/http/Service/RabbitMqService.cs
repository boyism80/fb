using Google.FlatBuffers;
using Http.Util;
using RabbitMQ.Client;

namespace Http.Service
{
    public class RabbitMqService
    {
        private readonly IConfiguration _configuration;
        private readonly Lazy<IConnection> _connection;
        private readonly Lazy<IModel> _channel;
        private readonly object _channelLock = new object();

        public RabbitMqService(IConfiguration configuration)
        {
            _configuration = configuration;
            _connection = new Lazy<IConnection>(GetConnection);
            _channel = new Lazy<IModel>(GetChannel);
        }

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

        private IModel GetChannel()
        {
            return _connection.Value.CreateModel();
        }

        public void Publish(IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            lock (_channelLock)
            {
                var channel = _channel.Value;
                channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
            }
        }

        public void Publish(string exchangeName, string routingKey, byte[] body, bool persistent = true)
        {
            if (body == null || body.Length == 0)
                return;

            lock (_channelLock)
            {
                var channel = _channel.Value;
                var props = channel.CreateBasicProperties();
                props.Persistent = persistent;
                channel.BasicPublish(exchangeName, routingKey, props, body);
            }
        }

        public void WithChannel(Action<IModel> action)
        {
            lock (_channelLock)
            {
                action(_channel.Value);
            }
        }
    }
}
