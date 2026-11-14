namespace Http.Service
{
    public abstract class ReadyBackgroundService : BackgroundService
    {
        private readonly HealthCheckService _health;

        protected ReadyBackgroundService(HealthCheckService health)
        {
            _health = health;
        }

        protected sealed override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            await _health.WhenReady(stoppingToken);
            await RunAsync(stoppingToken);
        }

        protected abstract Task RunAsync(CancellationToken stoppingToken);
    }
}
