using Google.FlatBuffers;
using Http.Redis;
using RabbitMQ.Client;
using RabbitMQ.Client.Events;
using Response = fb.protocol._internal.response;

namespace Http.Service
{
    public class ShutdownListenerService : BackgroundService
    {
        private readonly ILogger<ShutdownListenerService> _logger;
        private readonly IHostApplicationLifetime _lifetime;
        private readonly RedisService _redisService;
        private readonly IConfiguration _configuration;

        public ShutdownListenerService(IConfiguration config,
            ILogger<ShutdownListenerService> logger,
            IHostApplicationLifetime lifetime,
            RedisService redisService)
        {
            _configuration = config;
            _logger = logger;
            _lifetime = lifetime;
            _redisService = redisService;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var factory = new ConnectionFactory
            {
                HostName = _configuration["RabbitMQ:Internal:Host"],
                Port = int.Parse(_configuration["RabbitMQ:Internal:Port"]),
                UserName = _configuration["RabbitMQ:Internal:Uid"],
                Password = _configuration["RabbitMQ:Internal:Pwd"]
            };

            await using var connection = await factory.CreateConnectionAsync(stoppingToken);
            var channel = await connection.CreateChannelAsync(new CreateChannelOptions(false, false, null, null), stoppingToken);
            try
            {
                await channel.ExchangeDeclareAsync("amq.direct", ExchangeType.Direct, durable: true, autoDelete: false, arguments: null, passive: false, noWait: false, stoppingToken);

                string queueName;
                try
                {
                    var declareOk = await channel.QueueDeclareAsync(string.Empty, durable: false, exclusive: false, autoDelete: false, arguments: null, passive: false, noWait: false, stoppingToken);
                    queueName = declareOk.QueueName;
                    await channel.QueueBindAsync(queueName, "amq.direct", "fb.global", arguments: null, noWait: false, stoppingToken);
                }
                catch (RabbitMQ.Client.Exceptions.OperationInterruptedException ex)
                {
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

                var consumer = new AsyncEventingBasicConsumer(channel);
                consumer.ReceivedAsync += async (_, ea) =>
                {
                    try
                    {
                        var body = ea.Body.ToArray();
                        var reader = new BinaryReader(new MemoryStream(body));
                        var protocolType = (Response.FlatBufferProtocolType)reader.ReadInt32();
                        if (protocolType != Response.FlatBufferProtocolType.Shutdown)
                        {
                            return;
                        }

                        var size = reader.ReadInt32();
                        var bytes = reader.ReadBytes(size);

                        var type = Response.FlatBufferProtocolRouter.GetProtocolType(protocolType);
                        _ = Activator.CreateInstance(type, bytes) as IFlatBufferEx;

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
                            var isEmpty = (await redis.Connection.ScanKeysAsync("fb:heart-beat:*", 1)).Count == 0;
                            if (isEmpty)
                            {
                                _logger.LogInformation("All heart-beat keys deleted, shutting down.");
                                break;
                            }

                            await Task.Delay(TimeSpan.FromSeconds(1));
                        }

                        _lifetime.StopApplication();
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError(ex, "Error parsing shutdown message");
                    }
                };

                await channel.BasicConsumeAsync(queueName, autoAck: true, consumer, stoppingToken);

                try
                {
                    await Task.Delay(Timeout.InfiniteTimeSpan, stoppingToken);
                }
                catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
                {
                }
            }
            finally
            {
                try
                {
                    await channel.CloseAsync(cancellationToken: CancellationToken.None);
                }
                catch
                {
                    // Ignore on shutdown
                }

                await channel.DisposeAsync();
            }
        }
    }
}
