using Google.FlatBuffers;
using Http.Util;
using RabbitMQ.Client;
using System.Threading.Channels;

namespace Http.Service
{
    public class RabbitMqService : IHostedService, IAsyncDisposable
    {
        internal static readonly CreateChannelOptions PublishChannelOptions = new CreateChannelOptions(
            publisherConfirmationsEnabled: true,
            publisherConfirmationTrackingEnabled: true,
            outstandingPublisherConfirmationsRateLimiter: null,
            consumerDispatchConcurrency: null);

        private const int QueueCapacity = 10000;
        private static readonly TimeSpan PublishTimeout = TimeSpan.FromSeconds(2);
        private static readonly TimeSpan EnqueueTimeout = TimeSpan.FromSeconds(2);
        private static readonly TimeSpan ConfirmTimeout = TimeSpan.FromSeconds(5);

        private readonly IConfiguration _configuration;
        private readonly ILogger<RabbitMqService> _logger;
        private readonly Channel<PublishJob> _pending = Channel.CreateBounded<PublishJob>(new BoundedChannelOptions(QueueCapacity)
        {
            FullMode = BoundedChannelFullMode.Wait,
            SingleReader = true,
            SingleWriter = false
        });
        private readonly CancellationTokenSource _stopping = new();
        private IConnection _connection;
        private IChannel _channel;
        private Task _run = Task.CompletedTask;
        private int _stopOnce;
        private int _disposed;

        public RabbitMqService(IConfiguration configuration, ILogger<RabbitMqService> logger)
        {
            _configuration = configuration;
            _logger = logger;
        }

        public Task StartAsync(CancellationToken cancellationToken)
        {
            _run = Task.Run(() => PublishLoop(_stopping.Token));
            return Task.CompletedTask;
        }

        public async Task StopAsync(CancellationToken cancellationToken)
        {
            if (Interlocked.Exchange(ref _stopOnce, 1) != 0)
            {
                await _run.WaitAsync(cancellationToken);
                return;
            }

            _pending.Writer.TryComplete();
            try
            {
                await _run.WaitAsync(cancellationToken);
            }
            catch (OperationCanceledException)
            {
                _stopping.Cancel();
                await _run;
            }

            await CloseAsync();
        }

        public Task PublishAsync(IFlatBufferEx protocol, string exchangeName, string routeKey, CancellationToken cancellationToken = default)
        {
            var bytes = protocol.ToBytes();
            return PublishAsync(exchangeName, routeKey, bytes, persistent: false, cancellationToken);
        }

        public async Task PublishFanoutAsync(IFlatBufferEx protocol, string topic, uint homeWorld, CancellationToken cancellationToken = default)
        {
            foreach (var key in AmqpRoute.Fanout(topic, homeWorld))
            {
                await PublishAsync(protocol, AmqpRoute.Exchange, key, cancellationToken);
            }
        }

        // Returns after the broker confirms every route, so the caller can retry a publish that was lost.
        public async Task PublishFanoutConfirmedAsync(IFlatBufferEx protocol, string topic, uint homeWorld, CancellationToken cancellationToken = default)
        {
            var bytes = protocol.ToBytes();
            foreach (var key in AmqpRoute.Fanout(topic, homeWorld))
            {
                var sent = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
                await EnqueueAsync(new PublishJob(AmqpRoute.Exchange, key, bytes, false, null, null, sent), cancellationToken);
                await sent.Task.WaitAsync(ConfirmTimeout, cancellationToken);
            }
        }

        // The change is already saved, so a failed notification must not turn the response into an error:
        // the caller would undo a change that persisted.
        public async Task NotifySavedAsync(IFlatBufferEx protocol, string topic, uint homeWorld)
        {
            try
            {
                await PublishFanoutAsync(protocol, topic, homeWorld);
            }
            catch (Exception e)
            {
                _logger.LogError(e, "Saved change notification failed on topic {Topic}", topic);
            }
        }

        public async Task NotifySavedAsync(IFlatBufferEx protocol, string exchangeName, string routeKey)
        {
            try
            {
                await PublishAsync(protocol, exchangeName, routeKey);
            }
            catch (Exception e)
            {
                _logger.LogError(e, "Saved change notification failed on route {Route}", routeKey);
            }
        }

        public async Task PublishAsync(
            string exchangeName,
            string routingKey,
            byte[] body,
            bool persistent = true,
            CancellationToken cancellationToken = default,
            Func<IChannel, CancellationToken, Task> prepare = null,
            Func<Exception, Task> onFailure = null)
        {
            if (body == null || body.Length == 0)
            {
                return;
            }

            try
            {
                await EnqueueAsync(new PublishJob(exchangeName, routingKey, body, persistent, prepare, onFailure, null), cancellationToken);
            }
            catch (Exception ex)
            {
                if (onFailure == null)
                {
                    throw;
                }

                await onFailure(ex);
            }
        }

        // A full queue means the broker is not draining it; fail instead of holding the request open.
        private async Task EnqueueAsync(PublishJob job, CancellationToken cancellationToken)
        {
            using var full = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            full.CancelAfter(EnqueueTimeout);
            await _pending.Writer.WriteAsync(job, full.Token);
        }

        private async Task PublishLoop(CancellationToken stoppingToken)
        {
            try
            {
                await foreach (var job in _pending.Reader.ReadAllAsync(stoppingToken))
                {
                    try
                    {
                        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(stoppingToken);
                        timeout.CancelAfter(PublishTimeout);
                        await PublishJobAsync(job, timeout.Token);
                        job.Sent?.TrySetResult();
                    }
                    catch (Exception ex)
                    {
                        await InvokeFailure(job, ex);
                        if (stoppingToken.IsCancellationRequested)
                        {
                            break;
                        }
                    }
                }
            }
            catch (OperationCanceledException)
            {
            }

            while (_pending.Reader.TryRead(out var leftover))
            {
                await InvokeFailure(leftover, new OperationCanceledException("publish stopped before send"));
            }
        }

        private async Task InvokeFailure(PublishJob job, Exception ex)
        {
            if (job.Sent != null)
            {
                job.Sent.TrySetException(ex);
                return;
            }

            if (job.OnFailure == null)
            {
                _logger.LogError(ex, "Failed to publish {Exchange}/{RoutingKey}", job.Exchange, job.RoutingKey);
                return;
            }

            try
            {
                await job.OnFailure(ex);
            }
            catch (Exception callbackEx)
            {
                _logger.LogError(callbackEx, "Publish failure callback failed for {RoutingKey}", job.RoutingKey);
            }
        }

        private async Task PublishJobAsync(PublishJob job, CancellationToken cancellationToken)
        {
            await EnsureConnectedAsync(cancellationToken);
            if (_channel == null)
            {
                throw new InvalidOperationException("internal RabbitMQ channel is not open");
            }

            if (job.Prepare != null)
            {
                await job.Prepare(_channel, cancellationToken);
            }

            if (job.Persistent)
            {
                var props = new BasicProperties { Persistent = true };
                await _channel.BasicPublishAsync(job.Exchange, job.RoutingKey, mandatory: true, basicProperties: props, body: job.Body, cancellationToken: cancellationToken);
            }
            else
            {
                await _channel.BasicPublishAsync(job.Exchange, job.RoutingKey, job.Body, cancellationToken);
            }
        }

        private async Task EnsureConnectedAsync(CancellationToken cancellationToken)
        {
            if (_connection != null && _connection.IsOpen && _channel != null && _channel.IsOpen)
            {
                return;
            }

            await CloseAsync();

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
                Password = rabbitMqSection.GetValue<string>("Pwd"),
                RequestedConnectionTimeout = PublishTimeout
            };

            _connection = await factory.CreateConnectionAsync(cancellationToken);
            _channel = await _connection.CreateChannelAsync(PublishChannelOptions, cancellationToken);
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
            if (Interlocked.Exchange(ref _disposed, 1) != 0)
            {
                return;
            }

            await StopAsync(CancellationToken.None);
            _stopping.Dispose();
        }

        private readonly record struct PublishJob(
            string Exchange,
            string RoutingKey,
            byte[] Body,
            bool Persistent,
            Func<IChannel, CancellationToken, Task> Prepare,
            Func<Exception, Task> OnFailure,
            TaskCompletionSource Sent);
    }
}
