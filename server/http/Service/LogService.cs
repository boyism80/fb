using Newtonsoft.Json;
using RabbitMQ.Client;
using System.Text;

namespace Http.Service
{
    public class LogService
    {
        private readonly IConnection _connection = null;
        private readonly IModel _channel = null;
        private readonly string _serverId;
        private readonly string _serverName;
        private readonly uint _world;
        private readonly Random _random;
        private readonly ILogger<LogService> _logger;
        private readonly bool _enabled;

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
                _world = 0;
                _random = new Random();
                return;
            }

            // Read server identification from Log section
            _serverId = logConfig["ServerId"] ?? "0";
            _serverName = logConfig["ServerName"] ?? "unknown";

            // Read world from configuration (0 means unified, >0 means specific world)
            _world = (uint)configuration.GetValue<int>("World", 0);

            // Read RabbitMQ connection info from RabbitMQ.Log section
            var logSection = configuration.GetSection("RabbitMQ:Log");

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
                    timestamp = DateTimeOffset.UtcNow.ToString("yyyy-MM-dd HH:mm:ss"),
                    @event = eventType,
                    server_id = _serverId,
                    server_name = _serverName,
                    data = data
                };

                // Serialize to JSON string
                var jsonString = JsonConvert.SerializeObject(logEntry);

                // Convert to byte array
                var message = Encoding.UTF8.GetBytes(jsonString);

                var routingKey = GetRoutingKey();

                // Publish to RabbitMQ (using amq.direct exchange with routing key)
                _channel.BasicPublish(exchange: "amq.direct", routingKey: routingKey, basicProperties: null, body: message);
            }
            catch (Exception ex)
            {
                _logger?.LogWarning(ex, "Failed to write log: {EventType}", eventType);
            }
        }

        private string GetRoutingKey()
        {
            if (_world > 0)
                return $"fb.{_world}.log";
            return "fb.log";
        }

        public void Dispose()
        {
            _channel?.Close();
            _connection?.Close();
        }
    }
}
