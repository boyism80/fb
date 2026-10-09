using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Response = fb.protocol.@internal.response;

namespace Http.Service.Amqp
{
    [AmqpHandler("fb.global")]
    public sealed class ShutdownHandler : AmqpHandler<Response.Shutdown>
    {
        private readonly ILogger<ShutdownHandler> _logger;
        private readonly IHostApplicationLifetime _lifetime;
        private readonly ServerStateService _serverStateService;

        public ShutdownHandler(
            ILogger<ShutdownHandler> logger,
            IHostApplicationLifetime lifetime,
            ServerStateService serverStateService)
        {
            _logger = logger;
            _lifetime = lifetime;
            _serverStateService = serverStateService;
        }

        protected override async Task HandleAsync(Response.Shutdown message, CancellationToken cancellationToken)
        {
            _logger.LogWarning("Shutdown message received.");

            while (!cancellationToken.IsCancellationRequested)
            {
                if (await _serverStateService.HasRunningServers() == false)
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
