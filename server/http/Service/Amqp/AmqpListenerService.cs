using Google.FlatBuffers;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using RabbitMQ.Client;
using RabbitMQ.Client.Events;

namespace Http.Service.Amqp
{
    public sealed class AmqpListenerService : BackgroundService
    {
        private readonly IConfiguration _configuration;
        private readonly ILogger<AmqpListenerService> _logger;
        private readonly AmqpHandlerRegistry _registry;
        private readonly IServiceScopeFactory _scopeFactory;

        public AmqpListenerService(
            IConfiguration configuration,
            ILogger<AmqpListenerService> logger,
            AmqpHandlerRegistry registry,
            IServiceScopeFactory scopeFactory)
        {
            _configuration = configuration;
            _logger = logger;
            _registry = registry;
            _scopeFactory = scopeFactory;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            if (_registry.Handlers.Count == 0)
            {
                _logger.LogWarning("No AMQP handlers discovered; listener will not start");
                return;
            }

            var factory = new ConnectionFactory
            {
                HostName = _configuration["RabbitMQ:Internal:Host"],
                Port = int.Parse(_configuration["RabbitMQ:Internal:Port"]!),
                UserName = _configuration["RabbitMQ:Internal:Uid"],
                Password = _configuration["RabbitMQ:Internal:Pwd"]
            };

            await using var connection = await factory.CreateConnectionAsync(stoppingToken);
            var channel = await connection.CreateChannelAsync(new CreateChannelOptions(false, false, null, null), stoppingToken);
            try
            {
                await channel.ExchangeDeclareAsync(
                    "amq.direct",
                    ExchangeType.Direct,
                    durable: true,
                    autoDelete: false,
                    arguments: null,
                    passive: false,
                    noWait: false,
                    stoppingToken);

                // One exclusive queue bound to every distinct route key used by handlers.
                var declareOk = await channel.QueueDeclareAsync(
                    string.Empty,
                    durable: false,
                    exclusive: true,
                    autoDelete: true,
                    arguments: null,
                    passive: false,
                    noWait: false,
                    stoppingToken);
                var queueName = declareOk.QueueName;

                foreach (var routeKey in _registry.RouteKeys)
                {
                    await channel.QueueBindAsync(queueName, "amq.direct", routeKey, arguments: null, noWait: false, stoppingToken);
                    _logger.LogInformation("AMQP listener bound queue to {RouteKey}", routeKey);
                }

                var consumer = new AsyncEventingBasicConsumer(channel);
                consumer.ReceivedAsync += async (_, ea) =>
                {
                    try
                    {
                        await DispatchAsync(ea.RoutingKey, ea.Body.ToArray(), stoppingToken);
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError(ex, "Error handling AMQP message");
                    }
                };

                await channel.BasicConsumeAsync(queueName, autoAck: true, consumer, stoppingToken);
                _logger.LogInformation(
                    "AMQP listener started with {HandlerCount} handler(s)",
                    _registry.Handlers.Count);

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

        private async Task DispatchAsync(string routeKey, byte[] body, CancellationToken cancellationToken)
        {
            using var stream = new MemoryStream(body);
            using var reader = new BinaryReader(stream);
            var protocolType = reader.ReadInt32();
            var size = reader.ReadInt32();
            var bytes = reader.ReadBytes(size);

            if (!_registry.TryGet(routeKey, protocolType, out var registration))
                return;

            IFlatBufferEx message;
            try
            {
                message = Activator.CreateInstance(registration.MessageType, bytes) as IFlatBufferEx;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to deserialize AMQP message type {MessageType}", registration.MessageType.Name);
                return;
            }

            if (message == null)
            {
                _logger.LogError("Deserialized AMQP message was null for {MessageType}", registration.MessageType.Name);
                return;
            }

            await using var scope = _scopeFactory.CreateAsyncScope();
            var handler = (IAmqpHandler)scope.ServiceProvider.GetRequiredService(registration.HandlerType);
            await handler.HandleAsync(message, cancellationToken);
        }
    }
}
