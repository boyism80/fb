using Newtonsoft.Json;
using RabbitMQ.Client;
using System.Text;

namespace Http.Service
{
    public class LogService : IAsyncDisposable
    {
        private readonly IConfiguration _configuration;
        private IConnection _connection;
        private IChannel _channel;
        private readonly string _serverId;
        private readonly string _serverName;
        private readonly uint _world;
        private readonly ILogger<LogService> _logger;
        private readonly bool _enabled;
        private readonly SemaphoreSlim _mqLock = new(1, 1);

        public LogService(IConfiguration configuration, ILogger<LogService> logger = null)
        {
            _configuration = configuration;
            _logger = logger;

            var logConfig = configuration.GetSection("Log");
            _enabled = logConfig.GetValue<bool>("Enabled", true);

            if (!_enabled)
            {
                _serverId = string.Empty;
                _serverName = string.Empty;
                _world = 0;
                return;
            }

            _serverId = logConfig["ServerId"] ?? "0";
            _serverName = logConfig["ServerName"] ?? "unknown";
            _world = (uint)configuration.GetValue<int>("World", 0);
        }

        private async Task EnsureConnectedAsync(CancellationToken cancellationToken)
        {
            if (_connection != null && _connection.IsOpen && _channel != null && _channel.IsOpen)
            {
                return;
            }

            if (_channel != null)
            {
                try
                {
                    await _channel.CloseAsync(cancellationToken: cancellationToken);
                }
                catch
                {
                    // Ignore close errors on stale channel
                }

                await _channel.DisposeAsync();
                _channel = null;
            }

            if (_connection != null)
            {
                try
                {
                    await _connection.CloseAsync(cancellationToken: cancellationToken);
                }
                catch
                {
                    // Ignore close errors on stale connection
                }

                await _connection.DisposeAsync();
                _connection = null;
            }

            var logSection = _configuration.GetSection("RabbitMQ:Log");
            var factory = new ConnectionFactory
            {
                HostName = logSection["Host"],
                Port = logSection.GetValue<int>("Port"),
                UserName = logSection["Uid"],
                Password = logSection["Pwd"]
            };

            _connection = await factory.CreateConnectionAsync(cancellationToken);
            _channel = await _connection.CreateChannelAsync(new CreateChannelOptions(false, false, null, null), cancellationToken);
        }

        public async Task WriteAsync(string eventType, object data, CancellationToken cancellationToken = default)
        {
            if (!_enabled)
            {
                return;
            }

            await _mqLock.WaitAsync(cancellationToken);
            try
            {
                try
                {
                    await EnsureConnectedAsync(cancellationToken);
                }
                catch (Exception ex)
                {
                    var logSection = _configuration.GetSection("RabbitMQ:Log");
                    _logger?.LogWarning(ex, "Failed to connect to log RabbitMQ at {Host}:{Port}", logSection["Host"], logSection["Port"]);
                    return;
                }

                if (_channel == null)
                {
                    _logger?.LogWarning("AMQP connection not available, skipping log publish");
                    return;
                }

                var logEntry = new
                {
                    timestamp = DateTimeOffset.UtcNow.ToString("yyyy-MM-dd HH:mm:ss"),
                    @event = eventType,
                    server_id = _serverId,
                    server_name = _serverName,
                    data = data
                };

                var jsonString = JsonConvert.SerializeObject(logEntry);
                var message = Encoding.UTF8.GetBytes(jsonString);
                var routingKey = GetRoutingKey();

                await _channel.BasicPublishAsync("amq.direct", routingKey, message, cancellationToken);
            }
            catch (Exception ex)
            {
                _logger?.LogWarning(ex, "Failed to write log: {EventType}", eventType);
            }
            finally
            {
                _mqLock.Release();
            }
        }

        private string GetRoutingKey()
        {
            if (_world > 0)
            {
                return $"fb.{_world}.log";
            }

            return "fb.log";
        }

        public async ValueTask DisposeAsync()
        {
            await _mqLock.WaitAsync();
            try
            {
                if (_channel != null)
                {
                    try
                    {
                        await _channel.CloseAsync(cancellationToken: CancellationToken.None);
                    }
                    catch
                    {
                        // Ignore on shutdown
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
                        // Ignore on shutdown
                    }

                    await _connection.DisposeAsync();
                    _connection = null;
                }
            }
            finally
            {
                _mqLock.Release();
            }

            _mqLock.Dispose();
        }
    }
}
