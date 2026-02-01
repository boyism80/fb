using Google.FlatBuffers;
using Http.Redis;
using RabbitMQ.Client;
using RabbitMQ.Client.Events;
using Response = fb.protocol._internal.response;

namespace Http.Service
{
    /// <summary>
    /// Provides a background service that listens for shutdown messages via RabbitMQ.
    /// Monitors heart-beat keys in Redis and triggers graceful application shutdown when all services are ready.
    /// </summary>
    public class ShutdownListenerService : BackgroundService
    {
        private readonly ILogger<ShutdownListenerService> _logger;
        private readonly IHostApplicationLifetime _lifetime;
        private readonly IConnection _connection;
        private readonly IModel _channel;
        private readonly RedisService _redisService;
        private readonly string _queueName;

        /// <summary>
        /// Initializes a new instance of the <see cref="ShutdownListenerService"/> class.
        /// Sets up RabbitMQ connection and queue binding for shutdown message listening.
        /// </summary>
        /// <param name="config">The configuration containing RabbitMQ connection settings.</param>
        /// <param name="logger">The logger for recording shutdown events and errors.</param>
        /// <param name="lifetime">The application lifetime service for triggering shutdown.</param>
        /// <param name="redisService">The Redis service for monitoring heart-beat keys.</param>
        public ShutdownListenerService(IConfiguration config,
            ILogger<ShutdownListenerService> logger,
            IHostApplicationLifetime lifetime,
            RedisService redisService)
        {
            _logger = logger;
            _lifetime = lifetime;
            _redisService = redisService;

            var factory = new ConnectionFactory
            {
                HostName = config["RabbitMQ:Internal:Host"],
                Port = int.Parse(config["RabbitMQ:Internal:Port"]),
                UserName = config["RabbitMQ:Internal:Uid"],
                Password = config["RabbitMQ:Internal:Pwd"]
            };

            _connection = factory.CreateConnection();
            _channel = _connection.CreateModel();

            _channel.ExchangeDeclare("amq.direct", ExchangeType.Direct, durable: true);

            try
            {
                _queueName = _channel.QueueDeclare(string.Empty, durable: false, exclusive: false, autoDelete: false, arguments: null);
                _channel.QueueBind(_queueName, "amq.direct", "fb.global");
            }
            catch (RabbitMQ.Client.Exceptions.OperationInterruptedException ex)
            {
                // Check if it's a PRECONDITION_FAILED error (queue exists with different parameters)
                if (ex.ShutdownReason?.ReplyText?.Contains("PRECONDITION_FAILED") == true)
                {
                    _logger.LogError(ex,
                        "Failed to declare shutdown listener queue: Queue already exists with different parameters.");
                }
                else
                {
                    _logger.LogError(ex, "Failed to declare shutdown listener queue");
                }
                throw;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Unexpected error declaring shutdown listener queue");
                throw;
            }
        }

        /// <summary>
        /// Executes the background service that listens for shutdown messages.
        /// Processes FlatBuffer protocol messages and coordinates graceful shutdown based on heart-beat monitoring.
        /// </summary>
        /// <param name="stoppingToken">The cancellation token for stopping the service.</param>
        /// <returns>A task representing the asynchronous execution of the background service.</returns>
        protected override Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var consumer = new EventingBasicConsumer(_channel);
            consumer.Received += (model, ea) =>
            {
                try
                {
                    var reader = new BinaryReader(new MemoryStream(ea.Body.ToArray()));
                    var protocolType = (Response.FlatBufferProtocolType)reader.ReadInt32();
                    if (protocolType != Response.FlatBufferProtocolType.Shutdown)
                        return;

                    var size = reader.ReadInt32();
                    var bytes = reader.ReadBytes(size);

                    var type = Response.FlatBufferProtocolRouter.GetProtocolType(protocolType);
                    var protocol = Activator.CreateInstance(type, bytes) as IFlatBufferEx;

                    _logger.LogWarning("Shutdown message received.");

                    var redis = _redisService.GetUnifiedConnection();
                    if (redis == null)
                    {
                        _logger.LogError("unified Redis not available, shutting down immediately.");
                        _lifetime.StopApplication();
                        return;
                    }

                    while (true)
                    {
                        var isEmpty = redis.Connection.ScanKeysAsync("heart-beat:*", 1).Result.Count == 0;
                        if (isEmpty)
                        {
                            _logger.LogInformation("All heart-beat keys deleted, shutting down.");
                            break;
                        }

                        Thread.Sleep(TimeSpan.FromSeconds(1));
                    }
                    _lifetime.StopApplication(); // triggers graceful shutdown
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Error parsing shutdown message");
                }
            };

            _channel.BasicConsume(queue: _queueName, autoAck: true, consumer: consumer);
            return Task.CompletedTask;
        }

        /// <summary>
        /// Disposes of the RabbitMQ resources when the service is being disposed.
        /// Properly closes the channel and connection to prevent resource leaks.
        /// </summary>
        public override void Dispose()
        {
            _channel?.Close();
            _connection?.Close();
            base.Dispose();
        }
    }
}
