using Google.FlatBuffers;
using http.Util;
using RabbitMQ.Client;

namespace Internal.Service
{
    public class RabbitMqService
    {
        private readonly ConnectionFactory _factory;

        public RabbitMqService(IConfiguration configuration)
        {
            var section = configuration.GetSection("RabbitMQ");
            _factory = new ConnectionFactory() 
            {
                HostName = section.GetValue<string>("Host"), 
                Port = section.GetValue<int>("Port"), 
                UserName = section.GetValue<string>("Uid"), 
                Password = section.GetValue<string>("Pwd")
            };
        }

        public void Publish(IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            using var connection = _factory.CreateConnection();
            using var channel = connection.CreateModel();
            channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
        }
    }
}
