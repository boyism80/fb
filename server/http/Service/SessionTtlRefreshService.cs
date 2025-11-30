namespace Http.Service
{
    /// <summary>
    /// Provides a background service that periodically refreshes the TTL of user sessions in Redis.
    /// Uses an atomic Lua script to prevent redundant TTL updates across multiple service instances.
    /// </summary>
    public class SessionTtlRefreshService : BackgroundService
    {
        private readonly SessionService _sessionService;
        private readonly ILogger<SessionTtlRefreshService> _logger;
        private static readonly TimeSpan RefreshInterval = TimeSpan.FromSeconds(30);

        /// <summary>
        /// Initializes a new instance of the <see cref="SessionTtlRefreshService"/> class.
        /// </summary>
        /// <param name="sessionService">The session service for refreshing TTL.</param>
        /// <param name="logger">The logger for recording refresh operations.</param>
        public SessionTtlRefreshService(SessionService sessionService, ILogger<SessionTtlRefreshService> logger)
        {
            _sessionService = sessionService;
            _logger = logger;
        }

        /// <summary>
        /// Executes the background service that periodically refreshes session TTL.
        /// </summary>
        /// <param name="stoppingToken">The cancellation token for stopping the service.</param>
        /// <returns>A task representing the asynchronous execution of the background service.</returns>
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    await _sessionService.RefreshTTL();
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Error refreshing session TTL");
                }

                await Task.Delay(RefreshInterval, stoppingToken);
            }
        }
    }
}

