namespace Marketplace.Services
{
    public class MarketplaceDeliveryBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly ILogger<MarketplaceDeliveryBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromSeconds(30);
        // Rows younger than this are still being delivered by the request that inserted them.
        private static readonly TimeSpan PendingMinAge = TimeSpan.FromSeconds(30);
        private const int BatchSize = 100;

        public MarketplaceDeliveryBackgroundService(
            IServiceScopeFactory scopeFactory,
            ILogger<MarketplaceDeliveryBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    using var scope = _scopeFactory.CreateScope();
                    var dbContext = scope.ServiceProvider.GetRequiredService<Marketplace.Service.DbContext>();
                    var deliveryService = scope.ServiceProvider.GetRequiredService<MarketplaceDeliveryService>();

                    var pending = await dbContext.MarketplaceDelivery.GetPendingAsync(PendingMinAge, BatchSize);
                    foreach (var delivery in pending)
                    {
                        if (stoppingToken.IsCancellationRequested)
                            break;

                        await deliveryService.DeliverAsync(delivery);
                    }
                }
                catch (Exception e)
                {
                    _logger.LogError(e, "Error retrying marketplace deliveries");
                }

                await Task.Delay(ProcessingInterval, stoppingToken);
            }
        }
    }
}
