using Google.FlatBuffers;
using Http.Util;
using RabbitMQ.Client;
using System.Collections.Concurrent;

namespace Http.Service
{
    /// <summary>
    /// Provides RabbitMQ messaging service functionality for publishing messages.
    /// Manages RabbitMQ connections and channels per world for message publishing operations.
    /// </summary>
    public class RabbitMqService
    {
        private readonly IConfiguration _configuration;
        private readonly ConcurrentDictionary<string, IConnection> _connections = new();
        private readonly ConcurrentDictionary<string, IModel> _channels = new();

        /// <summary>
        /// Initializes a new instance of the <see cref="RabbitMqService"/> class.
        /// </summary>
        /// <param name="configuration">The application configuration containing RabbitMQ connection settings.</param>
        public RabbitMqService(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        /// <summary>
        /// Gets or creates a RabbitMQ connection for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>The RabbitMQ connection for the specified world.</returns>
        private IConnection GetConnection(uint world)
        {
            var worldKey = world == 0 ? "unified-global" : world.ToString();
            return _connections.GetOrAdd(worldKey, worldName =>
            {
                var rabbitMqSection = _configuration.GetSection($"RabbitMQ:{worldName}:Internal");
                if (rabbitMqSection == null || !rabbitMqSection.Exists())
                {
                    throw new Exception($"RabbitMQ configuration not found for world: {worldName}");
                }

                var factory = new ConnectionFactory()
                {
                    HostName = rabbitMqSection.GetValue<string>("Host"),
                    Port = rabbitMqSection.GetValue<int>("Port"),
                    UserName = rabbitMqSection.GetValue<string>("Uid"),
                    Password = rabbitMqSection.GetValue<string>("Pwd")
                };

                return factory.CreateConnection();
            });
        }

        /// <summary>
        /// Gets or creates a RabbitMQ channel for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>The RabbitMQ channel for the specified world.</returns>
        private IModel GetChannel(uint world)
        {
            var worldKey = world == 0 ? "unified-global" : world.ToString();
            return _channels.GetOrAdd(worldKey, worldName =>
            {
                var connection = GetConnection(world);
                return connection.CreateModel();
            });
        }

        /// <summary>
        /// Publishes a FlatBuffer protocol message to the specified exchange with routing key.
        /// Uses the RabbitMQ connection for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global. If null, publishes to all worlds.</param>
        /// <param name="protocol">The FlatBuffer protocol message to publish.</param>
        /// <param name="exchangeName">The name of the exchange to publish to.</param>
        /// <param name="routeKey">The routing key for message routing.</param>
        public void Publish(uint? world, IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            if (world == null)
            {
                Broadcast(protocol, exchangeName, routeKey);
                return;
            }

            var channel = GetChannel(world.Value);
            channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
        }

        /// <summary>
        /// Broadcasts a FlatBuffer protocol message to all worlds.
        /// Publishes the message to the specified exchange with routing key for each configured world.
        /// </summary>
        /// <param name="protocol">The FlatBuffer protocol message to publish.</param>
        /// <param name="exchangeName">The name of the exchange to publish to.</param>
        /// <param name="routeKey">The routing key for message routing.</param>
        public void Broadcast(IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            var rabbitMqSection = _configuration.GetSection("RabbitMQ");
            var worlds = rabbitMqSection.GetChildren()
                .Where(child => uint.TryParse(child.Key, out _) || child.Key == "unified-global")
                .Select(child => child.Key == "unified-global" ? 0 : uint.Parse(child.Key));

            foreach (var world in worlds)
            {
                try
                {
                    var channel = GetChannel(world);
                    channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
                }
                catch (Exception ex)
                {
                    // Log error but continue publishing to other worlds
                    // Note: In production, you might want to inject ILogger here
                    System.Diagnostics.Debug.WriteLine($"Failed to publish to world {world}: {ex.Message}");
                }
            }
        }
    }
}
