using Google.FlatBuffers;
using Http.Util;
using RabbitMQ.Client;

namespace Http.Service
{
    public class RabbitMqService : IAsyncDisposable
    {
        private readonly IConfiguration _configuration;
        private readonly SemaphoreSlim _channelLock = new(1, 1);
        private IConnection _connection;
        private IChannel _channel;

        public RabbitMqService(IConfiguration configuration)
        {
            _configuration = configuration;
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

            var rabbitMqSection = _configuration.GetSection("RabbitMQ:Internal");
            if (rabbitMqSection == null || !rabbitMqSection.Exists())
            {
                throw new Exception("RabbitMQ configuration not found for unified-global");
            }

            var factory = new ConnectionFactory
            {
                HostName = rabbitMqSection.GetValue<string>("Host"),
                Port = rabbitMqSection.GetValue<int>("Port"),
                UserName = rabbitMqSection.GetValue<string>("Uid"),
                Password = rabbitMqSection.GetValue<string>("Pwd")
            };

            _connection = await factory.CreateConnectionAsync(cancellationToken);
            _channel = await _connection.CreateChannelAsync(new CreateChannelOptions(false, false, null, null), cancellationToken);
        }

        public async Task PublishAsync(IFlatBufferEx protocol, string exchangeName, string routeKey, CancellationToken cancellationToken = default)
        {
            var bytes = protocol.ToBytes();
            await PublishAsync(exchangeName, routeKey, bytes, persistent: false, cancellationToken);
        }

        public async Task PublishAsync(string exchangeName, string routingKey, byte[] body, bool persistent = true, CancellationToken cancellationToken = default)
        {
            if (body == null || body.Length == 0)
            {
                return;
            }

            await _channelLock.WaitAsync(cancellationToken);
            try
            {
                await EnsureConnectedAsync(cancellationToken);
                if (persistent)
                {
                    var props = new BasicProperties { Persistent = true };
                    await _channel.BasicPublishAsync(exchangeName, routingKey, mandatory: false, basicProperties: props, body: body, cancellationToken: cancellationToken);
                }
                else
                {
                    await _channel.BasicPublishAsync(exchangeName, routingKey, body, cancellationToken);
                }
            }
            finally
            {
                _channelLock.Release();
            }
        }

        public async Task WithChannelAsync(Func<IChannel, Task> action, CancellationToken cancellationToken = default)
        {
            await _channelLock.WaitAsync(cancellationToken);
            try
            {
                await EnsureConnectedAsync(cancellationToken);
                await action(_channel);
            }
            finally
            {
                _channelLock.Release();
            }
        }

        public async ValueTask DisposeAsync()
        {
            await _channelLock.WaitAsync();
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
                _channelLock.Release();
            }

            _channelLock.Dispose();
        }
    }
}
