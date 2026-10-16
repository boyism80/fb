using Newtonsoft.Json;
using RabbitMQ.Client;
using System.Text;
using System.Threading.Channels;

namespace Http.Service
{
    public class LogService : IHostedService, IAsyncDisposable
    {
        private const int QueueCapacity = 20000;
        private static readonly TimeSpan PublishTimeout = TimeSpan.FromSeconds(2);

        private readonly IConfiguration _configuration;
        private readonly ILogger<LogService> _logger;
        private readonly bool _enabled;
        private readonly string _serverId;
        private readonly string _serverName;
        private readonly uint _world;
        private readonly Channel<LogEntry> _pending = Channel.CreateBounded<LogEntry>(new BoundedChannelOptions(QueueCapacity)
        {
            FullMode = BoundedChannelFullMode.DropOldest,
            SingleReader = true,
            SingleWriter = false
        });
        private readonly CancellationTokenSource _stopping = new();
        private IConnection _connection;
        private IChannel _channel;
        private Task _run = Task.CompletedTask;
        private long _dropped;
        private int _publishFailures;

        public LogService(IConfiguration configuration, ILogger<LogService> logger = null)
        {
            _configuration = configuration;
            _logger = logger;

            var logConfig = configuration.GetSection("Log");
            _enabled = logConfig.GetValue<bool>("Enabled", true);
            _serverName = logConfig["ServerName"] ?? "unknown";
            _world = (uint)configuration.GetValue<int>("World", 0);

            var serverId = logConfig["ServerId"];
            if (string.IsNullOrWhiteSpace(serverId) || serverId == "0")
                _serverId = Environment.MachineName;
            else
                _serverId = serverId;
        }

        public Task StartAsync(CancellationToken cancellationToken)
        {
            if (_enabled)
                _run = Task.Run(() => PublishLoop(_stopping.Token));
            return Task.CompletedTask;
        }

        public async Task StopAsync(CancellationToken cancellationToken)
        {
            _stopping.Cancel();
            _pending.Writer.TryComplete();
            try
            {
                await _run.WaitAsync(PublishTimeout, cancellationToken);
            }
            catch (Exception)
            {
            }

            await CloseAsync();
        }

        public Task WriteAsync(string eventType, object data, CancellationToken cancellationToken = default)
        {
            if (!_enabled)
                return Task.CompletedTask;

            if (_pending.Reader.Count >= QueueCapacity)
            {
                var dropped = Interlocked.Increment(ref _dropped);
                if (dropped == 1 || dropped % 1000 == 0)
                    _logger?.LogWarning("Dropped {Dropped} log entries because the publish queue is full", dropped);
            }

            _pending.Writer.TryWrite(new LogEntry(DateTimeOffset.UtcNow.ToString("yyyy-MM-dd HH:mm:ss"), eventType, data));
            return Task.CompletedTask;
        }

        private async Task PublishLoop(CancellationToken stoppingToken)
        {
            try
            {
                await foreach (var entry in _pending.Reader.ReadAllAsync(stoppingToken))
                {
                    try
                    {
                        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(stoppingToken);
                        timeout.CancelAfter(PublishTimeout);
                        await PublishAsync(entry, timeout.Token);
                        Interlocked.Exchange(ref _publishFailures, 0);
                    }
                    catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
                    {
                        break;
                    }
                    catch (Exception ex)
                    {
                        var failures = Interlocked.Increment(ref _publishFailures);
                        if (failures == 1 || failures % 100 == 0)
                            _logger?.LogWarning(ex, "Failed to publish log {EventType} ({Failures} consecutive failures)", entry.EventType, failures);
                    }
                }
            }
            catch (OperationCanceledException)
            {
            }
        }

        private async Task PublishAsync(LogEntry entry, CancellationToken cancellationToken)
        {
            await EnsureConnectedAsync(cancellationToken);
            if (_channel == null)
                throw new InvalidOperationException("log RabbitMQ channel is not open");

            var logEntry = new
            {
                timestamp = entry.Timestamp,
                @event = entry.EventType,
                server_id = _serverId,
                server_name = _serverName,
                data = entry.Data
            };
            var message = Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(logEntry));
            await _channel.BasicPublishAsync("amq.direct", RoutingKey(), message, cancellationToken);
        }

        private async Task EnsureConnectedAsync(CancellationToken cancellationToken)
        {
            if (_connection != null && _connection.IsOpen && _channel != null && _channel.IsOpen)
                return;

            await CloseAsync();

            var logSection = _configuration.GetSection("RabbitMQ:Log");
            var factory = new ConnectionFactory
            {
                HostName = logSection["Host"],
                Port = logSection.GetValue<int>("Port"),
                UserName = logSection["Uid"],
                Password = logSection["Pwd"],
                RequestedConnectionTimeout = PublishTimeout
            };

            _connection = await factory.CreateConnectionAsync(cancellationToken);
            _channel = await _connection.CreateChannelAsync(RabbitMqService.PublishChannelOptions, cancellationToken);
        }

        private string RoutingKey()
        {
            if (_world > 0)
                return $"fb.{_world}.log";
            return "fb.log";
        }

        private async Task CloseAsync()
        {
            if (_channel != null)
            {
                try
                {
                    await _channel.CloseAsync(cancellationToken: CancellationToken.None);
                }
                catch
                {
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
                }

                await _connection.DisposeAsync();
                _connection = null;
            }
        }

        public async ValueTask DisposeAsync()
        {
            await StopAsync(CancellationToken.None);
            _stopping.Dispose();
        }

        private readonly record struct LogEntry(string Timestamp, string EventType, object Data);
    }
}
