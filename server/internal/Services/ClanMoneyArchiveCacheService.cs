using Http.Service;

namespace Internal.Services
{
    public class ClanMoneyArchiveCacheService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly ILogger<ClanMoneyArchiveCacheService> _logger;
        private static readonly TimeSpan Interval = TimeSpan.FromMinutes(10);

        public ClanMoneyArchiveCacheService(IServiceScopeFactory scopeFactory, ILogger<ClanMoneyArchiveCacheService> logger)
        {
            _scopeFactory = scopeFactory;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (stoppingToken.IsCancellationRequested == false)
            {
                try
                {
                    await using var scope = _scopeFactory.CreateAsyncScope();
                    await scope.ServiceProvider.GetRequiredService<DbContext>().ClanMoneyRequestArchive.DropCachedHashesAsync();
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Clan money archive cache drop failed");
                }

                try
                {
                    await Task.Delay(Interval, stoppingToken);
                }
                catch (OperationCanceledException)
                {
                    break;
                }
            }
        }
    }
}
