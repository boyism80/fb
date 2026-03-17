using Log.Repository;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using RabbitMQ.Client;
using System.Text;
using System.Text.Json;

namespace Log.Worker
{
    public class LogConsumerService : BackgroundService
    {
        private readonly IConfiguration _configuration;
        private readonly IServiceScopeFactory _serviceScopeFactory;
        private readonly ILogger<LogConsumerService> _logger;
        private IConnection _connection;
        private IModel _channel;
        private readonly List<string> _queueNames = new();
        private static readonly TimeSpan ProcessInterval = TimeSpan.FromSeconds(10);
        private const int BatchSize = 1000;
        private const string ExchangeName = "amq.direct";
        private readonly uint _world;

        public LogConsumerService(
            IConfiguration configuration,
            IServiceScopeFactory serviceScopeFactory,
            ILogger<LogConsumerService> logger)
        {
            _configuration = configuration;
            _serviceScopeFactory = serviceScopeFactory;
            _logger = logger;
            _world = (uint)_configuration.GetValue<int>("World", 0);
            if (_world == 0)
            {
                throw new Exception("Log consumer service requires a world > 0. Unified-global is not supported.");
            }
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            _logger.LogInformation("Log Consumer Service starting");

            // Retry connection loop until successful or cancelled
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    await ConnectToRabbitMQAsync(stoppingToken);
                    _logger.LogInformation("Log Consumer Service started");

                    // Main processing loop
                    while (!stoppingToken.IsCancellationRequested)
                    {
                        try
                        {
                            await ProcessLogsAsync(stoppingToken);
                        }
                        catch (Exception ex)
                        {
                            _logger.LogError(ex, "Error processing logs");
                        }

                        await Task.Delay(ProcessInterval, stoppingToken);
                    }
                    break; // Exit retry loop if cancellation requested
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Failed to connect to RabbitMQ, retrying in 5 seconds...");
                    await DisconnectFromRabbitMQAsync(); // Clean up failed connection
                    await Task.Delay(TimeSpan.FromSeconds(5), stoppingToken);
                }
            }

            // Process remaining messages during shutdown
            _logger.LogInformation("Graceful shutdown initiated, processing remaining messages...");
            await ProcessRemainingMessagesAsync();

            // Clean shutdown
            await DisconnectFromRabbitMQAsync();
            _logger.LogInformation("Log Consumer Service stopped, all messages processed");
        }

        private async Task ProcessRemainingMessagesAsync()
        {
            if (_channel == null)
                return;

            var totalProcessed = 0;
            while (true)
            {
                var processedCount = 0;
                var allLogs = new List<JsonElement>();
                var messagesToAck = new List<(string QueueName, ulong DeliveryTag)>();

                foreach (var queueName in _queueNames)
                {
                    while (true)
                    {
                        var result = _channel.BasicGet(queueName, autoAck: false);
                        if (result == null)
                            break;

                        try
                        {
                            var body = result.Body.ToArray();
                            var jsonString = Encoding.UTF8.GetString(body);
                            using var doc = JsonDocument.Parse(jsonString);
                            ParseLogMessage(doc.RootElement, allLogs);
                            messagesToAck.Add((queueName, result.DeliveryTag));
                            processedCount++;
                        }
                        catch (Exception ex)
                        {
                            _logger.LogWarning(ex, "Failed to parse log message from queue {QueueName}", queueName);
                            _channel.BasicAck(result.DeliveryTag, false);
                        }
                    }
                }

                if (processedCount == 0)
                {
                    break;
                }

                totalProcessed += processedCount;

                foreach (var (queueName, deliveryTag) in messagesToAck)
                {
                    try
                    {
                        _channel.BasicAck(deliveryTag, false);
                    }
                    catch (Exception ex)
                    {
                        _logger.LogWarning(ex, $"Failed to acknowledge message from queue {queueName}");
                    }
                }

                if (allLogs.Count > 0)
                {
                    try
                    {
                        using var scope = _serviceScopeFactory.CreateScope();
                        var logRepository = scope.ServiceProvider.GetRequiredService<LogRepository>();
                        await logRepository.BulkInsertAsync(allLogs);
                        _logger.LogInformation($"Processed {allLogs.Count} remaining log entries");
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError(ex, $"Failed to bulk insert {allLogs.Count} log entries during shutdown");
                    }
                }
            }

            _logger.LogInformation($"Graceful shutdown complete, processed {totalProcessed} remaining messages");
        }

        private async Task ConnectToRabbitMQAsync(CancellationToken cancellationToken)
        {
            var section = _configuration.GetSection("RabbitMQ:Log");
            var hostName = section.GetValue<string>("Host");
            var port = section.GetValue<int>("Port");
            var userName = section.GetValue<string>("Uid");
            var password = section.GetValue<string>("Pwd");

            _logger.LogInformation($"Attempting to connect to RabbitMQ at {hostName}:{port}");

            var factory = new ConnectionFactory
            {
                HostName = hostName,
                Port = port,
                UserName = userName,
                Password = password
            };

            _connection = factory.CreateConnection();
            _logger.LogInformation("RabbitMQ connection established");

            _channel = _connection.CreateModel();

            // Declare exchange (should already exist, but ensure it's durable)
            _channel.ExchangeDeclare(ExchangeName, ExchangeType.Direct, durable: true);

            // Single queue per world: game server publishes batched log arrays to fb.{world}.log
            var routingKey = $"fb.{_world}.log";
            var queueName = routingKey;

            try
            {
                _channel.QueueDeclare(
                    queue: queueName,
                    durable: true,
                    exclusive: false,
                    autoDelete: false,
                    arguments: null);

                _channel.QueueBind(queueName, ExchangeName, routingKey);
                _queueNames.Add(queueName);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to declare or bind queue '{QueueName}' to routing key '{RoutingKey}'", queueName, routingKey);
                throw;
            }

            _logger.LogInformation("Connected to RabbitMQ and declared queue {QueueName}", queueName);
            await Task.CompletedTask;
        }

        private static void ParseLogMessage(JsonElement root, List<JsonElement> allLogs)
        {
            if (root.ValueKind == JsonValueKind.Array)
            {
                foreach (var element in root.EnumerateArray())
                    allLogs.Add(element.Clone());
            }
            else if (root.ValueKind == JsonValueKind.Object)
            {
                allLogs.Add(root.Clone());
            }
        }

        private async Task ProcessLogsAsync(CancellationToken cancellationToken)
        {
            if (_channel == null)
                return;

            var allLogs = new List<JsonElement>();
            var messagesToAck = new List<(string QueueName, ulong DeliveryTag)>();

            // Process messages from the auto-generated queue
            // This queue is bound to all log routing keys (fb.log.0 to fb.log.127)
            foreach (var queueName in _queueNames)
            {
                var messageCount = 0;

                // Collect messages from this queue (up to BatchSize per queue)
                while (messageCount < BatchSize)
                {
                    var result = _channel.BasicGet(queueName, autoAck: false);
                    if (result == null)
                        break;

                    try
                    {
                        var body = result.Body.ToArray();
                        var jsonString = Encoding.UTF8.GetString(body);
                        using var doc = JsonDocument.Parse(jsonString);
                        ParseLogMessage(doc.RootElement, allLogs);
                        messagesToAck.Add((queueName, result.DeliveryTag));
                        messageCount++;
                    }
                    catch (Exception ex)
                    {
                        _logger.LogWarning(ex, "Failed to parse log message from queue {QueueName}", queueName);
                        _channel.BasicAck(result.DeliveryTag, false);
                    }
                }
            }

            if (allLogs.Count == 0)
            {
                return;
            }

            foreach (var (queueName, deliveryTag) in messagesToAck)
            {
                try
                {
                    _channel.BasicAck(deliveryTag, false);
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, $"Failed to acknowledge message from queue {queueName}");
                }
            }

            // Bulk insert all collected logs into MySQL
            if (allLogs.Count > 0)
            {
                try
                {
                    using var scope = _serviceScopeFactory.CreateScope();
                    var logRepository = scope.ServiceProvider.GetRequiredService<LogRepository>();
                    await logRepository.BulkInsertAsync(allLogs);
                    _logger.LogInformation($"Processed {allLogs.Count} log entries");
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, $"Failed to bulk insert {allLogs.Count} log entries");
                }
            }
        }

        private async Task DisconnectFromRabbitMQAsync()
        {
            try
            {
                _channel?.Close();
                _connection?.Close();
                _queueNames.Clear();
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error disconnecting from RabbitMQ");
            }

            await Task.CompletedTask;
        }

        public override void Dispose()
        {
            DisconnectFromRabbitMQAsync().Wait();
            base.Dispose();
        }
    }
}
