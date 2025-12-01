using Http.Service;
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
    /// <summary>
    /// Background service that consumes log messages from RabbitMQ queues and stores them in the database.
    /// Subscribes to all log queues (fb.log.0 to fb.log.127) and periodically processes messages in batches.
    /// </summary>
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
        private const string QueueNamePrefix = "fb.log.";

        /// <summary>
        /// Initializes a new instance of the <see cref="LogConsumerService"/> class.
        /// </summary>
        /// <param name="configuration">The application configuration containing RabbitMQ connection settings.</param>
        /// <param name="serviceScopeFactory">The service scope factory for creating scoped dependencies.</param>
        /// <param name="logger">The logger instance.</param>
        public LogConsumerService(
            IConfiguration configuration,
            IServiceScopeFactory serviceScopeFactory,
            ILogger<LogConsumerService> logger)
        {
            _configuration = configuration;
            _serviceScopeFactory = serviceScopeFactory;
            _logger = logger;
        }

        /// <summary>
        /// Executes the background service logic.
        /// Connects to RabbitMQ, declares queues, and periodically processes log messages.
        /// </summary>
        /// <param name="stoppingToken">Cancellation token to stop the service.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            try
            {
                await ConnectToRabbitMQAsync(stoppingToken);
                _logger.LogInformation("Log Consumer Service started");

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
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Fatal error in Log Consumer Service");
            }
            finally
            {
                await DisconnectFromRabbitMQAsync();
                _logger.LogInformation("Log Consumer Service stopped");
            }
        }

        /// <summary>
        /// Connects to RabbitMQ and sets up queues for log consumption.
        /// </summary>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        private async Task ConnectToRabbitMQAsync(CancellationToken cancellationToken)
        {
            var section = _configuration.GetSection("RabbitMQ");
            var factory = new ConnectionFactory
            {
                HostName = section.GetValue<string>("Host"),
                Port = section.GetValue<int>("Port"),
                UserName = section.GetValue<string>("Uid"),
                Password = section.GetValue<string>("Pwd")
            };

            _connection = factory.CreateConnection();
            _channel = _connection.CreateModel();

            // Declare exchange (should already exist, but ensure it's durable)
            _channel.ExchangeDeclare(ExchangeName, ExchangeType.Direct, durable: true);

            // Get queue size from configuration
            var queueSize = _configuration.GetValue<int>("RabbitMQ:QueueSize", 128);

            // Declare and bind all log queues
            // In Direct exchange, queue name and routing key are the same (e.g., "fb.log.0")
            for (int i = 0; i < queueSize; i++)
            {
                var queueName = $"{QueueNamePrefix}{i}";
                var routingKey = queueName; // For Direct exchange, routing key equals queue name
                _queueNames.Add(queueName);

                // Declare queue as durable to persist messages
                _channel.QueueDeclare(queueName, durable: true, exclusive: false, autoDelete: false, arguments: null);

                // Bind queue to exchange with routing key (same as queue name for Direct exchange)
                _channel.QueueBind(queueName, ExchangeName, routingKey);
            }

            _logger.LogInformation($"Connected to RabbitMQ and subscribed to {queueSize} log queues");
            await Task.CompletedTask;
        }

        /// <summary>
        /// Processes log messages from all queues in batches using BasicGet.
        /// </summary>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        private async Task ProcessLogsAsync(CancellationToken cancellationToken)
        {
            if (_channel == null)
                return;

            var allLogs = new List<JsonElement>();
            var messagesToAck = new List<(string QueueName, ulong DeliveryTag)>();

            // Process messages from all queues
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
                        allLogs.Add(doc.RootElement.Clone());
                        messagesToAck.Add((queueName, result.DeliveryTag));
                        messageCount++;
                    }
                    catch (Exception ex)
                    {
                        _logger.LogWarning(ex, $"Failed to parse log message from queue {queueName}");
                        // Acknowledge even if parsing fails to avoid reprocessing
                        _channel.BasicAck(result.DeliveryTag, false);
                    }
                }
            }

            // Acknowledge all processed messages
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

        /// <summary>
        /// Disconnects from RabbitMQ and cleans up resources.
        /// </summary>
        /// <returns>A task representing the asynchronous operation.</returns>
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

        /// <summary>
        /// Disposes of the RabbitMQ resources when the service is being disposed.
        /// </summary>
        public override void Dispose()
        {
            DisconnectFromRabbitMQAsync().Wait();
            base.Dispose();
        }
    }
}

