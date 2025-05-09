using Google.FlatBuffers;
using Http.Util;
using RabbitMQ.Client;

namespace Http.Service
{
    public class RabbitMqService
    {
        private readonly IConnection _connection;
        private readonly IModel _channel;

        public RabbitMqService(IConfiguration configuration)
        {
            var section = configuration.GetSection("RabbitMQ");
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

        public void Publish(IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            _channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
        }
    }
}
