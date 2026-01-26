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
        private readonly IConfiguration _configuration;
        private static readonly TimeSpan RefreshInterval = TimeSpan.FromSeconds(30);

        /// <summary>
        /// Initializes a new instance of the <see cref="SessionTtlRefreshService"/> class.
        /// </summary>
        /// <param name="sessionService">The session service for refreshing TTL.</param>
        /// <param name="logger">The logger for recording refresh operations.</param>
        /// <param name="configuration">The configuration for retrieving section information.</param>
        public SessionTtlRefreshService(SessionService sessionService, ILogger<SessionTtlRefreshService> logger, IConfiguration configuration)
        {
            _sessionService = sessionService;
            _logger = logger;
            _configuration = configuration;
        }

        /// <summary>
        /// Executes the background service that periodically refreshes session TTL.
        /// </summary>
        /// <param name="stoppingToken">The cancellation token for stopping the service.</param>
        /// <returns>A task representing the asynchronous execution of the background service.</returns>
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            // Get all game sections from configuration
            var mysqlSection = _configuration.GetSection("ConnectionStrings:MySql");
            var sections = mysqlSection.GetChildren().Select(x => x.Key).ToList();

            while (!stoppingToken.IsCancellationRequested)
            {
                foreach (var section in sections)
                {
                    try
                    {
                        await _sessionService.RefreshTTL(section);
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError(ex, $"Error refreshing session TTL for section {section}");
                    }
                }

                await Task.Delay(RefreshInterval, stoppingToken);
            }
        }
    }
}

