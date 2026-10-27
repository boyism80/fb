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
        private record PendingMessage(string QueueName, ulong DeliveryTag, IReadOnlyBasicProperties Properties, byte[] Body, List<JsonElement> Logs);

        private readonly IConfiguration _configuration;
        private readonly IServiceScopeFactory _serviceScopeFactory;
        private readonly ILogger<LogConsumerService> _logger;
        private IConnection _connection;
        private IChannel _channel;
        private IChannel _publisher;
        private readonly List<string> _queueNames = new();
        private static readonly TimeSpan ProcessInterval = TimeSpan.FromSeconds(10);
        private const int BatchSize = 8;
        private const string ExchangeName = "amq.direct";
        private const string DeadLetterExchangeName = "fb.log.dlx";
        private const string DeadLetterQueueName = "fb.log.dlq";
        private const string DeadLetterRouteExchangeName = "fb.log.dlx.route";
        private const string DeadQueueSuffix = ".dead";
        private readonly List<PendingMessage> _pending = new();
        private readonly uint _world;
        private readonly bool _crossLog;

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

            _crossLog = _configuration.GetValue<bool>("CrossLog", false);
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            _logger.LogInformation("Log Consumer Service starting");

            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    await ConnectToRabbitMQAsync(stoppingToken);
                    _logger.LogInformation("Log Consumer Service started");

                    // A channel closed by the broker is not recovered by the client, so reconnect instead of reusing it.
                    while (!stoppingToken.IsCancellationRequested && _channel.IsOpen)
                    {
                        try
                        {
                            var batchFull = await ProcessLogsAsync(stoppingToken);
                            if (batchFull)
                            {
                                continue;
                            }
                        }
                        catch (Exception ex)
                        {
                            _logger.LogError(ex, "Error processing logs");
                        }

                        await Task.Delay(ProcessInterval, stoppingToken);
                    }

                    if (stoppingToken.IsCancellationRequested)
                    {
                        break;
                    }

                    _logger.LogWarning("Log channel closed, reconnecting");
                    await DisconnectFromRabbitMQAsync();
                }
                catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
                {
                    break;
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Failed to connect to RabbitMQ, retrying in 5 seconds...");
                    await DisconnectFromRabbitMQAsync();
                    try
                    {
                        await Task.Delay(TimeSpan.FromSeconds(5), stoppingToken);
                    }
                    catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
                    {
                        break;
                    }
                }
            }

            _logger.LogInformation("Graceful shutdown initiated, processing remaining messages...");
            using var drain = new CancellationTokenSource(TimeSpan.FromSeconds(30));
            try
            {
                await ProcessRemainingMessagesAsync(drain.Token);
            }
            catch (OperationCanceledException)
            {
                _logger.LogWarning("Log drain timed out; unacknowledged messages remain in RabbitMQ");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Log drain stopped; unacknowledged messages remain in RabbitMQ");
            }

            await DisconnectFromRabbitMQAsync();
            _logger.LogInformation("Log Consumer Service stopped");
        }

        private async Task ProcessRemainingMessagesAsync(CancellationToken cancellationToken)
        {
            if (_channel == null)
            {
                return;
            }

            var totalProcessed = 0;
            while (true)
            {
                var processedCount = 0;
                var allLogs = new List<JsonElement>();
                var messagesToAck = new List<(string QueueName, ulong DeliveryTag)>();

                foreach (var queueName in _queueNames)
                {
                    var messageCount = 0;
                    while (messageCount < BatchSize)
                    {
                        var result = await _channel.BasicGetAsync(queueName, autoAck: false, cancellationToken);
                        if (result == null)
                        {
                            break;
                        }

                        try
                        {
                            var body = result.Body.ToArray();
                            var jsonString = Encoding.UTF8.GetString(body);
                            using var doc = JsonDocument.Parse(jsonString);
                            ParseLogMessage(doc.RootElement, allLogs);
                            messagesToAck.Add((queueName, result.DeliveryTag));
                            processedCount++;
                            messageCount++;
                        }
                        catch (Exception ex)
                        {
                            _logger.LogWarning(ex, "Failed to parse log message from queue {QueueName}", queueName);
                            await _channel.BasicAckAsync(result.DeliveryTag, false, cancellationToken);
                        }
                    }
                }

                if (processedCount == 0)
                {
                    break;
                }

                var inserted = true;
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
                        inserted = false;
                    }
                }

                foreach (var (queueName, deliveryTag) in messagesToAck)
                {
                    try
                    {
                        if (inserted)
                        {
                            await _channel.BasicAckAsync(deliveryTag, false, cancellationToken);
                        }
                        else
                        {
                            await _channel.BasicNackAsync(deliveryTag, multiple: false, requeue: true, cancellationToken);
                        }
                    }
                    catch (Exception ex)
                    {
                        _logger.LogWarning(ex, $"Failed to acknowledge message from queue {queueName}");
                    }
                }
                if (!inserted)
                {
                    _logger.LogWarning("Log drain stopped after insert failure; messages were requeued");
                    break;
                }
                totalProcessed += processedCount;
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

            _connection = await factory.CreateConnectionAsync(cancellationToken);
            _logger.LogInformation("RabbitMQ connection established");

            _channel = await _connection.CreateChannelAsync(new CreateChannelOptions(false, false, null, null), cancellationToken);
            _publisher = await _connection.CreateChannelAsync(new CreateChannelOptions(true, true, null, null), cancellationToken);

            await _channel.ExchangeDeclareAsync(ExchangeName, ExchangeType.Direct, durable: true, autoDelete: false, arguments: null, passive: false, noWait: false, cancellationToken);

            await _channel.ExchangeDeclareAsync(DeadLetterExchangeName, ExchangeType.Fanout, durable: true, autoDelete: false, arguments: null, passive: false, noWait: false, cancellationToken);
            await _channel.QueueDeclareAsync(
                queue: DeadLetterQueueName,
                durable: true,
                exclusive: false,
                autoDelete: false,
                arguments: new Dictionary<string, object> { ["x-queue-type"] = "quorum" },
                passive: false,
                noWait: false,
                cancellationToken: cancellationToken);
            await _channel.QueueUnbindAsync(DeadLetterQueueName, DeadLetterExchangeName, string.Empty, arguments: null, cancellationToken);
            await _channel.ExchangeDeclareAsync(DeadLetterRouteExchangeName, ExchangeType.Direct, durable: true, autoDelete: false, arguments: null, passive: false, noWait: false, cancellationToken);
            await _channel.ExchangeBindAsync(DeadLetterRouteExchangeName, DeadLetterExchangeName, string.Empty, arguments: null, noWait: false, cancellationToken);

            var routingKeys = new List<string> { $"fb.{_world}.log" };
            if (_crossLog)
            {
                routingKeys.Add("fb.log");
            }

            foreach (var routingKey in routingKeys)
            {
                var queueName = routingKey;

                try
                {
                    await _channel.QueueDeclareAsync(
                        queue: queueName,
                        durable: true,
                        exclusive: false,
                        autoDelete: false,
                        arguments: new Dictionary<string, object> { ["x-queue-type"] = "quorum" },
                        passive: false,
                        noWait: false,
                        cancellationToken: cancellationToken);

                    await _channel.QueueBindAsync(queueName, ExchangeName, routingKey, arguments: null, noWait: false, cancellationToken);
                    await DeclareDeadQueueAsync(_channel, queueName, cancellationToken);
                    _queueNames.Add(queueName);
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Failed to declare or bind queue '{QueueName}' to routing key '{RoutingKey}'", queueName, routingKey);
                    throw;
                }

                _logger.LogInformation("Connected to RabbitMQ and declared queue {QueueName}", queueName);
            }

            await MoveLegacyDeadLettersAsync(cancellationToken);
        }

        private static async Task DeclareDeadQueueAsync(IChannel channel, string queueName, CancellationToken cancellationToken)
        {
            var deadQueueName = queueName + DeadQueueSuffix;
            await channel.QueueDeclareAsync(
                queue: deadQueueName,
                durable: true,
                exclusive: false,
                autoDelete: false,
                arguments: new Dictionary<string, object> { ["x-queue-type"] = "quorum", ["x-delivery-limit"] = int.MaxValue },
                passive: false,
                noWait: false,
                cancellationToken: cancellationToken);
            await channel.QueueBindAsync(deadQueueName, DeadLetterRouteExchangeName, queueName, arguments: null, noWait: false, cancellationToken);
        }

        private async Task MoveLegacyDeadLettersAsync(CancellationToken cancellationToken)
        {
            var count = (await _channel.QueueDeclarePassiveAsync(DeadLetterQueueName, cancellationToken)).MessageCount;
            if (count == 0)
            {
                return;
            }

            var moved = 0;
            for (var i = 0; i < count; i++)
            {
                var result = await _channel.BasicGetAsync(DeadLetterQueueName, autoAck: false, cancellationToken);
                if (result == null)
                {
                    break;
                }

                var headers = result.BasicProperties.Headers;
                if (headers == null || !headers.TryGetValue("x-first-death-queue", out var value) || value is not byte[] raw)
                {
                    _logger.LogError("Dropping legacy dead letter without x-first-death-queue: {Body}", Encoding.UTF8.GetString(result.Body.Span));
                    await _channel.BasicAckAsync(result.DeliveryTag, false, cancellationToken);
                    continue;
                }

                var queueName = Encoding.UTF8.GetString(raw);
                await DeclareDeadQueueAsync(_publisher, queueName, cancellationToken);
                await _publisher.BasicPublishAsync(DeadLetterRouteExchangeName, queueName, true, new BasicProperties(result.BasicProperties), result.Body, cancellationToken);
                await _channel.BasicAckAsync(result.DeliveryTag, false, cancellationToken);
                moved++;
            }

            _logger.LogInformation("Moved {Count} legacy dead letters to per-queue dead queues", moved);
        }

        private static void ParseLogMessage(JsonElement root, List<JsonElement> allLogs)
        {
            if (root.ValueKind == JsonValueKind.Array)
            {
                foreach (var element in root.EnumerateArray())
                {
                    allLogs.Add(element.Clone());
                }
            }
            else if (root.ValueKind == JsonValueKind.Object)
            {
                allLogs.Add(root.Clone());
            }
        }

        private async Task<bool> ProcessLogsAsync(CancellationToken cancellationToken)
        {
            if (_channel == null)
            {
                return false;
            }

            var batchFull = false;
            if (_pending.Count == 0)
            {
                foreach (var queueName in _queueNames)
                {
                    var messageCount = 0;

                    while (messageCount < BatchSize)
                    {
                        var result = await _channel.BasicGetAsync(queueName, autoAck: false, cancellationToken);
                        if (result == null)
                        {
                            break;
                        }

                        try
                        {
                            var body = result.Body.ToArray();
                            var logs = new List<JsonElement>();
                            using var doc = JsonDocument.Parse(body);
                            ParseLogMessage(doc.RootElement, logs);
                            _pending.Add(new PendingMessage(queueName, result.DeliveryTag, result.BasicProperties, body, logs));
                            messageCount++;
                        }
                        catch (Exception ex)
                        {
                            _logger.LogWarning(ex, "Failed to parse log message from queue {QueueName}", queueName);
                            await _channel.BasicAckAsync(result.DeliveryTag, false, cancellationToken);
                        }
                    }
                    batchFull |= messageCount == BatchSize;
                }
            }

            if (_pending.Count == 0)
            {
                return false;
            }

            using var scope = _serviceScopeFactory.CreateScope();
            var logRepository = scope.ServiceProvider.GetRequiredService<LogRepository>();
            var logCount = _pending.Sum(x => x.Logs.Count);
            try
            {
                await logRepository.BulkInsertAsync(_pending.SelectMany(x => x.Logs));
                _logger.LogInformation($"Processed {logCount} log entries");
                foreach (var message in _pending)
                {
                    await AckAsync(message, cancellationToken);
                }
                _pending.Clear();
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, $"Failed to bulk insert {logCount} log entries, inserting each message alone");
                var failed = new List<PendingMessage>();
                foreach (var message in _pending)
                {
                    try
                    {
                        await logRepository.BulkInsertAsync(message.Logs);
                        await AckAsync(message, cancellationToken);
                    }
                    catch (Exception)
                    {
                        failed.Add(message);
                    }
                }
                _pending.Clear();

                if (failed.Count > 0 && await logRepository.PingAsync())
                {
                    foreach (var message in failed)
                    {
                        _logger.LogError($"Moving a log message that fails on a reachable database to {message.QueueName}{DeadQueueSuffix}");
                        await _publisher.BasicPublishAsync(DeadLetterRouteExchangeName, message.QueueName, true, new BasicProperties(message.Properties), message.Body, cancellationToken);
                        await AckAsync(message, cancellationToken);
                    }
                }
                else if (failed.Count > 0)
                {
                    _logger.LogWarning($"Log database is unreachable, holding {failed.Count} messages for retry");
                    _pending.AddRange(failed);
                    return false;
                }
            }

            await ReplayDeadLettersAsync(logRepository, cancellationToken);
            return batchFull;
        }

        private async Task ReplayDeadLettersAsync(LogRepository logRepository, CancellationToken cancellationToken)
        {
            foreach (var queueName in _queueNames)
            {
                var deadQueueName = queueName + DeadQueueSuffix;
                var count = (await _channel.QueueDeclarePassiveAsync(deadQueueName, cancellationToken)).MessageCount;
                var failed = new List<PendingMessage>();
                var replayed = 0;
                for (var i = 0; i < count; i++)
                {
                    var result = await _channel.BasicGetAsync(deadQueueName, autoAck: false, cancellationToken);
                    if (result == null)
                    {
                        break;
                    }

                    var message = new PendingMessage(deadQueueName, result.DeliveryTag, result.BasicProperties, result.Body.ToArray(), new List<JsonElement>());
                    try
                    {
                        using var doc = JsonDocument.Parse(message.Body);
                        ParseLogMessage(doc.RootElement, message.Logs);
                    }
                    catch (Exception ex)
                    {
                        _logger.LogWarning(ex, "Failed to parse log message from queue {QueueName}", deadQueueName);
                        await AckAsync(message, cancellationToken);
                        continue;
                    }

                    try
                    {
                        await logRepository.BulkInsertAsync(message.Logs);
                        await AckAsync(message, cancellationToken);
                        replayed++;
                    }
                    catch (Exception)
                    {
                        failed.Add(message);
                    }
                }

                if (failed.Count > 0 && await logRepository.PingAsync())
                {
                    foreach (var message in failed)
                    {
                        try
                        {
                            await logRepository.BulkInsertAsync(message.Logs);
                            replayed++;
                        }
                        catch (Exception ex)
                        {
                            _logger.LogError(ex, "Dropping a log message that keeps failing on a reachable database: {Body}", Encoding.UTF8.GetString(message.Body));
                        }
                        await AckAsync(message, cancellationToken);
                    }
                }
                else if (failed.Count > 0)
                {
                    foreach (var message in failed)
                    {
                        try
                        {
                            await _channel.BasicNackAsync(message.DeliveryTag, multiple: false, requeue: true, cancellationToken);
                        }
                        catch (Exception ex)
                        {
                            _logger.LogWarning(ex, $"Failed to requeue message to {deadQueueName}");
                        }
                    }
                }

                if (replayed > 0)
                {
                    _logger.LogInformation($"Replayed {replayed} dead-lettered log messages from {deadQueueName}");
                }
            }
        }

        private async Task AckAsync(PendingMessage message, CancellationToken cancellationToken)
        {
            try
            {
                await _channel.BasicAckAsync(message.DeliveryTag, false, cancellationToken);
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, $"Failed to acknowledge message from queue {message.QueueName}");
            }
        }

        private async Task DisconnectFromRabbitMQAsync()
        {
            try
            {
                if (_publisher != null)
                {
                    try
                    {
                        await _publisher.CloseAsync(cancellationToken: CancellationToken.None);
                    }
                    catch
                    {
                    }

                    await _publisher.DisposeAsync();
                    _publisher = null;
                }

                if (_channel != null)
                {
                    try
                    {
                        await _channel.CloseAsync(cancellationToken: CancellationToken.None);
                    }
                    catch
                    {
                        // Ignore
                    }

                    await _channel.DisposeAsync();
                    _channel = null;
                }

                if (_connection != null)
                {
                    try
                    {
                        await _connection.CloseAsync(cancellationToken: CancellationToken.None);
                    }
                    catch
                    {
                        // Ignore
                    }

                    await _connection.DisposeAsync();
                    _connection = null;
                }

                _queueNames.Clear();
                _pending.Clear();
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error disconnecting from RabbitMQ");
            }
        }
    }
}
