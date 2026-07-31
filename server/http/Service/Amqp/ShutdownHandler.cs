using Http.Redis;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Response = fb.protocol._internal.response;

namespace Http.Service.Amqp
{
    [AmqpHandler("fb.global")]
    public sealed class ShutdownHandler : AmqpHandler<Response.Shutdown>
    {
        private readonly ILogger<ShutdownHandler> _logger;
        private readonly IHostApplicationLifetime _lifetime;
        private readonly RedisService _redisService;

        public ShutdownHandler(
            ILogger<ShutdownHandler> logger,
            IHostApplicationLifetime lifetime,
            RedisService redisService)
        {
            _logger = logger;
            _lifetime = lifetime;
            _redisService = redisService;
        }

        protected override async Task HandleAsync(Response.Shutdown message, CancellationToken cancellationToken)
        {
            _logger.LogWarning("Shutdown message received.");

            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
            {
                _logger.LogError("unified Redis not available, shutting down immediately.");
                _lifetime.StopApplication();
                return;
            }

            while (!cancellationToken.IsCancellationRequested)
            {
                var isEmpty = (await redis.Connection.ScanKeysAsync("fb:heart-beat:*", 1)).Count == 0;
                if (isEmpty)
                {
                    _logger.LogInformation("All heart-beat keys deleted, shutting down.");
                    break;
                }

                await Task.Delay(TimeSpan.FromSeconds(1), cancellationToken);
            }

            _lifetime.StopApplication();
        }
    }
}
