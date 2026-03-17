namespace Http.Service
{
    public class SessionTtlRefreshService : BackgroundService
    {
        private readonly SessionService _sessionService;
        private readonly RedisService _redisService;
        private readonly ILogger<SessionTtlRefreshService> _logger;
        private static readonly TimeSpan RefreshInterval = TimeSpan.FromSeconds(30);

        public SessionTtlRefreshService(SessionService sessionService, RedisService redisService, ILogger<SessionTtlRefreshService> logger)
        {
            _sessionService = sessionService;
            _redisService = redisService;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var worlds = _redisService.GetConfiguredWorlds();

            while (!stoppingToken.IsCancellationRequested)
            {
                foreach (var world in worlds)
                {
                    try
                    {
                        await _sessionService.RefreshTTL(world);
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError(ex, $"Error refreshing session TTL for world {world}");
                    }
                }

                await Task.Delay(RefreshInterval, stoppingToken);
            }
        }
    }
}
