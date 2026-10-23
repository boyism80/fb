using Http.Service;

namespace Marketplace.Services
{
    public class MarketplaceParcelBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly HealthCheckService _health;
        private readonly ILogger<MarketplaceParcelBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromSeconds(30);
        // Rows younger than this are still being sent by the request that inserted them.
        private static readonly TimeSpan PendingMinAge = TimeSpan.FromSeconds(30);
        private const int BatchSize = 100;

        public MarketplaceParcelBackgroundService(
            IServiceScopeFactory scopeFactory,
            HealthCheckService health,
            ILogger<MarketplaceParcelBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _health = health;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (_health.IsReady == false && stoppingToken.IsCancellationRequested == false)
            {
                await Task.Delay(TimeSpan.FromSeconds(1), stoppingToken);
            }

            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    using var scope = _scopeFactory.CreateScope();
                    var dbContext = scope.ServiceProvider.GetRequiredService<Marketplace.Service.DbContext>();
                    var parcelService = scope.ServiceProvider.GetRequiredService<MarketplaceParcelService>();

                    var pending = await dbContext.MarketplaceParcel.GetPendingAsync(PendingMinAge, BatchSize);
                    foreach (var parcel in pending)
                    {
                        if (stoppingToken.IsCancellationRequested)
                        {
                            break;
                        }

                        await parcelService.SendAsync(parcel);
                    }
                }
                catch (Exception e)
                {
                    _logger.LogError(e, "Error retrying marketplace parcels");
                }

                await Task.Delay(ProcessingInterval, stoppingToken);
            }
        }
    }
}
