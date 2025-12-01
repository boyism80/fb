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
            _logger.LogInformation("Log Consumer Service starting");

            // Retry connection loop until successful or cancelled
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    _logger.LogInformation("ExecuteAsync: Attempting to connect to RabbitMQ");
                    await ConnectToRabbitMQAsync(stoppingToken);
                    _logger.LogInformation("Log Consumer Service started");

                    // Main processing loop
                    while (!stoppingToken.IsCancellationRequested)
                    {
                        try
                        {
                            _logger.LogDebug("ExecuteAsync: Starting log processing cycle");
                            await ProcessLogsAsync(stoppingToken);
                            _logger.LogDebug("ExecuteAsync: Completed log processing cycle");
                        }
                        catch (Exception ex)
                        {
                            _logger.LogError(ex, "ExecuteAsync: Error processing logs");
                        }

                        _logger.LogDebug("ExecuteAsync: Waiting {Interval} seconds before next processing cycle", ProcessInterval.TotalSeconds);
                        await Task.Delay(ProcessInterval, stoppingToken);
                    }
                    break; // Exit retry loop if cancellation requested
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "ExecuteAsync: Failed to connect to RabbitMQ, retrying in 5 seconds...");
                    await DisconnectFromRabbitMQAsync(); // Clean up failed connection
                    await Task.Delay(TimeSpan.FromSeconds(5), stoppingToken);
                }
            }

            // Clean shutdown
            await DisconnectFromRabbitMQAsync();
            _logger.LogInformation("Log Consumer Service stopped");
        }

        /// <summary>
        /// Connects to RabbitMQ and sets up queues for log consumption.
        /// </summary>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        private async Task ConnectToRabbitMQAsync(CancellationToken cancellationToken)
        {
            var section = _configuration.GetSection("RabbitMQ");
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

            _logger.LogInformation("ConnectToRabbitMQAsync: Creating RabbitMQ connection");
            _connection = factory.CreateConnection();
            _logger.LogInformation("ConnectToRabbitMQAsync: RabbitMQ connection established");

            _logger.LogInformation("ConnectToRabbitMQAsync: Creating RabbitMQ channel");
            _channel = _connection.CreateModel();
            _logger.LogInformation("ConnectToRabbitMQAsync: RabbitMQ channel created");

            // Declare exchange (should already exist, but ensure it's durable)
            _logger.LogInformation("ConnectToRabbitMQAsync: Declaring exchange {ExchangeName}", ExchangeName);
            _channel.ExchangeDeclare(ExchangeName, ExchangeType.Direct, durable: true);
            _logger.LogInformation("ConnectToRabbitMQAsync: Exchange {ExchangeName} declared", ExchangeName);

            // Get queue size from configuration
            var queueSize = _configuration.GetValue<int>("RabbitMQ:QueueSize", 128);

            // Declare and bind all log queues
            // In Direct exchange, queue name and routing key are the same (e.g., "fb.log.0")
            _logger.LogInformation("ConnectToRabbitMQAsync: Declaring and binding {QueueSize} log queues", queueSize);
            for (int i = 0; i < queueSize; i++)
            {
                var queueName = $"{QueueNamePrefix}{i}";
                var routingKey = queueName; // For Direct exchange, routing key equals queue name
                _queueNames.Add(queueName);

                try
                {
                    _logger.LogDebug("ConnectToRabbitMQAsync: Declaring queue {QueueName}", queueName);
                    // Declare queue as durable to persist messages
                    _channel.QueueDeclare(queueName, durable: true, exclusive: false, autoDelete: false, arguments: null);
                    _logger.LogDebug("ConnectToRabbitMQAsync: Queue {QueueName} declared", queueName);

                    _logger.LogDebug("ConnectToRabbitMQAsync: Binding queue {QueueName} to exchange {ExchangeName} with routing key {RoutingKey}",
                                    queueName, ExchangeName, routingKey);
                    // Bind queue to exchange with routing key (same as queue name for Direct exchange)
                    _channel.QueueBind(queueName, ExchangeName, routingKey);
                    _logger.LogDebug("ConnectToRabbitMQAsync: Queue {QueueName} bound successfully", queueName);
                }
                catch (RabbitMQ.Client.Exceptions.OperationInterruptedException ex)
                {
                    // Check if it's a PRECONDITION_FAILED error (queue exists with different parameters)
                    if (ex.ShutdownReason?.ReplyText?.Contains("PRECONDITION_FAILED") == true)
                    {
                        _logger.LogError(ex,
                            "Failed to declare queue '{QueueName}': Queue already exists with different parameters.",
                            queueName);
                    }
                    else
                    {
                        _logger.LogError(ex, "Failed to declare queue '{QueueName}'", queueName);
                    }
                    throw;
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Unexpected error declaring queue '{QueueName}'", queueName);
                    throw;
                }
            }

            _logger.LogInformation("ConnectToRabbitMQAsync: Connected to RabbitMQ and subscribed to {QueueSize} log queues", queueSize);
            await Task.CompletedTask;
        }

        /// <summary>
        /// Processes log messages from all queues in batches using BasicGet.
        /// </summary>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        private async Task ProcessLogsAsync(CancellationToken cancellationToken)
        {
            _logger.LogDebug("ProcessLogsAsync: Starting log processing cycle");
            
            if (_channel == null)
            {
                _logger.LogWarning("ProcessLogsAsync: Channel is null, cannot process logs");
                return;
            }

            var allLogs = new List<JsonElement>();
            var messagesToAck = new List<(string QueueName, ulong DeliveryTag)>();

            _logger.LogDebug("ProcessLogsAsync: Processing messages from {QueueCount} queues", _queueNames.Count);

            // Process messages from all queues
            foreach (var queueName in _queueNames)
            {
                var messageCount = 0;

                // Collect messages from this queue (up to BatchSize per queue)
                while (messageCount < BatchSize)
                {
                    try
                    {
                        var result = _channel.BasicGet(queueName, autoAck: false);
                        if (result == null)
                            break;

                        _logger.LogDebug("ProcessLogsAsync: Retrieved message from queue {QueueName}, deliveryTag={DeliveryTag}",
                                        queueName, result.DeliveryTag);

                        try
                        {
                            var body = result.Body.ToArray();
                            _logger.LogDebug("ProcessLogsAsync: Message body size={Size} bytes from queue {QueueName}",
                                            body.Length, queueName);
                            
                            var jsonString = Encoding.UTF8.GetString(body);
                            _logger.LogDebug("ProcessLogsAsync: Parsing JSON message from queue {QueueName}, length={Length}",
                                            queueName, jsonString.Length);
                            
                            using var doc = JsonDocument.Parse(jsonString);
                            allLogs.Add(doc.RootElement.Clone());
                            messagesToAck.Add((queueName, result.DeliveryTag));
                            messageCount++;
                            
                            _logger.LogDebug("ProcessLogsAsync: Successfully parsed message from queue {QueueName}, total_collected={Count}",
                                            queueName, allLogs.Count);
                        }
                        catch (Exception ex)
                        {
                            _logger.LogWarning(ex,
                                "ProcessLogsAsync: Failed to parse log message from queue {QueueName}, deliveryTag={DeliveryTag}",
                                queueName, result.DeliveryTag);
                            // Acknowledge even if parsing fails to avoid reprocessing
                            _channel.BasicAck(result.DeliveryTag, false);
                        }
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError(ex, "ProcessLogsAsync: Error retrieving message from queue {QueueName}", queueName);
                        break;
                    }
                }

                if (messageCount > 0)
                {
                    _logger.LogDebug("ProcessLogsAsync: Collected {Count} messages from queue {QueueName}",
                                    messageCount, queueName);
                }
            }

            _logger.LogDebug("ProcessLogsAsync: Total messages collected: {Count}", allLogs.Count);

            // Acknowledge all processed messages
            foreach (var (queueName, deliveryTag) in messagesToAck)
            {
                try
                {
                    _channel.BasicAck(deliveryTag, false);
                    _logger.LogDebug("ProcessLogsAsync: Acknowledged message from queue {QueueName}, deliveryTag={DeliveryTag}",
                                    queueName, deliveryTag);
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex,
                        "ProcessLogsAsync: Failed to acknowledge message from queue {QueueName}, deliveryTag={DeliveryTag}",
                        queueName, deliveryTag);
                }
            }

            // Bulk insert all collected logs into MySQL
            if (allLogs.Count > 0)
            {
                _logger.LogInformation("ProcessLogsAsync: Starting bulk insert of {Count} log entries", allLogs.Count);
                try
                {
                    using var scope = _serviceScopeFactory.CreateScope();
                    var logRepository = scope.ServiceProvider.GetRequiredService<LogRepository>();
                    await logRepository.BulkInsertAsync(allLogs);
                    _logger.LogInformation("ProcessLogsAsync: Successfully processed {Count} log entries", allLogs.Count);
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex,
                        "ProcessLogsAsync: Failed to bulk insert {Count} log entries",
                        allLogs.Count);
                }
            }
            else
            {
                _logger.LogDebug("ProcessLogsAsync: No log entries to process in this cycle");
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

