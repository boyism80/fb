using Google.FlatBuffers;
using Http.Util;
using RabbitMQ.Client;
using System.Collections.Concurrent;

namespace Http.Service
{
    /// <summary>
    /// Provides RabbitMQ messaging service functionality for publishing messages.
    /// Manages RabbitMQ connections and channels per section for message publishing operations.
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
        /// Gets or creates a RabbitMQ connection for the specified section.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "section-2").</param>
        /// <returns>The RabbitMQ connection for the specified section.</returns>
        private IConnection GetConnection(string section)
        {
            return _connections.GetOrAdd(section, sectionName =>
            {
                var rabbitMqSection = _configuration.GetSection($"RabbitMQ:{sectionName}:Internal");
                if (rabbitMqSection == null || !rabbitMqSection.Exists())
                {
                    throw new Exception($"RabbitMQ configuration not found for section: {sectionName}");
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
        /// Gets or creates a RabbitMQ channel for the specified section.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "section-2").</param>
        /// <returns>The RabbitMQ channel for the specified section.</returns>
        private IModel GetChannel(string section)
        {
            return _channels.GetOrAdd(section, sectionName =>
            {
                var connection = GetConnection(sectionName);
                return connection.CreateModel();
            });
        }

        /// <summary>
        /// Publishes a FlatBuffer protocol message to the specified exchange with routing key.
        /// Uses the RabbitMQ connection for the specified section.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "section-2"). If null, publishes to all sections.</param>
        /// <param name="protocol">The FlatBuffer protocol message to publish.</param>
        /// <param name="exchangeName">The name of the exchange to publish to.</param>
        /// <param name="routeKey">The routing key for message routing.</param>
        public void Publish(string section, IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            if (string.IsNullOrEmpty(section))
            {
                Broadcast(protocol, exchangeName, routeKey);
                return;
            }

            var channel = GetChannel(section);
            channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
        }

        /// <summary>
        /// Broadcasts a FlatBuffer protocol message to all sections.
        /// Publishes the message to the specified exchange with routing key for each configured section.
        /// </summary>
        /// <param name="protocol">The FlatBuffer protocol message to publish.</param>
        /// <param name="exchangeName">The name of the exchange to publish to.</param>
        /// <param name="routeKey">The routing key for message routing.</param>
        public void Broadcast(IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            var rabbitMqSection = _configuration.GetSection("RabbitMQ");
            var sections = rabbitMqSection.GetChildren()
                .Select(child => child.Key);

            foreach (var section in sections)
            {
                try
                {
                    var channel = GetChannel(section);
                    channel.BasicPublish(exchange: exchangeName, routingKey: routeKey, basicProperties: null, body: protocol.ToBytes());
                }
                catch (Exception ex)
                {
                    // Log error but continue publishing to other sections
                    // Note: In production, you might want to inject ILogger here
                    System.Diagnostics.Debug.WriteLine($"Failed to publish to section {section}: {ex.Message}");
                }
            }
        }
    }
}
