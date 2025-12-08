using Newtonsoft.Json;
using RabbitMQ.Client;
using System.Text;

namespace Http.Service
{
    /// <summary>
    /// Provides logging service functionality for publishing log entries to RabbitMQ.
    /// Implements the same functionality as C++ log_collector, sending logs to RabbitMQ exchange with routing keys.
    /// </summary>
    public class LogService
    {
        private readonly IConnection _connection = null;
        private readonly IModel _channel = null;
        private readonly string _serverId;
        private readonly string _serverName;
        private readonly int _queueSize;
        private readonly Random _random;
        private readonly ILogger<LogService> _logger;
        private readonly bool _enabled;

        /// <summary>
        /// Initializes a new instance of the <see cref="LogService"/> class.
        /// Establishes connection to RabbitMQ log server using configuration settings.
        /// </summary>
        /// <param name="configuration">The application configuration containing RabbitMQ and Log settings.</param>
        /// <param name="logger">The logger instance for error logging.</param>
        public LogService(IConfiguration configuration, ILogger<LogService> logger = null)
        {
            _logger = logger;

            // Check if logging is enabled
            var logConfig = configuration.GetSection("Log");
            _enabled = logConfig.GetValue<bool>("Enabled", true);

            if (!_enabled)
            {
                _serverId = string.Empty;
                _serverName = string.Empty;
                _queueSize = 128;
                _random = new Random();
                return;
            }

            // Read server identification from Log section
            _serverId = logConfig["ServerId"] ?? "0";
            _serverName = logConfig["ServerName"] ?? "unknown";

            // Read RabbitMQ connection info from RabbitMQ.Log section
            var logSection = configuration.GetSection("RabbitMQ:Log");
            _queueSize = logSection.GetValue<int>("QueueSize", 128);

            try
            {
                var factory = new ConnectionFactory
                {
                    HostName = logSection["Host"],
                    Port = logSection.GetValue<int>("Port"),
                    UserName = logSection["Uid"],
                    Password = logSection["Pwd"]
                };

                _connection = factory.CreateConnection();
                _channel = _connection.CreateModel();
            }
            catch (Exception ex)
            {
                _logger?.LogWarning(ex, "Failed to connect to log RabbitMQ at {Host}:{Port}", logSection["Host"], logSection["Port"]);
                _connection = null;
                _channel = null;
            }

            _random = new Random();
        }

        /// <summary>
        /// Writes a log entry to RabbitMQ with the specified event type and data.
        /// Creates a JSON log entry and publishes it to RabbitMQ using a random routing key.
        /// </summary>
        /// <param name="eventType">The type of event being logged (e.g., "account_create", "item_gain").</param>
        /// <param name="data">The data object to include in the log entry. Will be serialized to JSON.</param>
        public void Write(string eventType, object data)
        {
            if (!_enabled)
                return;

            try
            {
                if (_channel == null)
                {
                    _logger?.LogWarning("AMQP connection not available, skipping log publish");
                    return;
                }

                // Create log entry JSON (same format as C++ log_collector)
                var logEntry = new
                {
                    timestamp = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds(),
                    @event = eventType,
                    server_id = _serverId,
                    server_name = _serverName,
                    data = data
                };

                // Serialize to JSON string
                var jsonString = JsonConvert.SerializeObject(logEntry);

                // Convert to byte array
                var message = Encoding.UTF8.GetBytes(jsonString);

                // Select random routing key (fb.log.{0-queue_size-1})
                var routingKey = SelectRandomRoutingKey();

                // Publish to RabbitMQ (using amq.direct exchange with routing key)
                _channel.BasicPublish(exchange: "amq.direct", routingKey: routingKey, basicProperties: null, body: message);
            }
            catch (Exception ex)
            {
                _logger?.LogWarning(ex, "Failed to write log: {EventType}", eventType);
            }
        }

        /// <summary>
        /// Selects a random routing key for log message distribution.
        /// </summary>
        /// <returns>A routing key in the format "fb.log.{0-queue_size-1}".</returns>
        private string SelectRandomRoutingKey()
        {
            var queueIndex = _random.Next(0, _queueSize);
            return $"fb.log.{queueIndex}";
        }

        /// <summary>
        /// Disposes of the RabbitMQ resources when the service is being disposed.
        /// Properly closes the channel and connection to prevent resource leaks.
        /// </summary>
        public void Dispose()
        {
            _channel?.Close();
            _connection?.Close();
        }
    }
}

