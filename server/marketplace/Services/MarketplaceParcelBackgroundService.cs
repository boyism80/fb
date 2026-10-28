using System.Threading.Channels;
using Http.Service;

namespace Marketplace.Services
{
    public class MarketplaceParcelBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly HealthCheckService _health;
        private readonly ILogger<MarketplaceParcelBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromSeconds(30);
        private const int BatchSize = 100;

        private readonly Channel<bool> _wake = Channel.CreateBounded<bool>(
            new BoundedChannelOptions(1) { FullMode = BoundedChannelFullMode.DropWrite });

        public MarketplaceParcelBackgroundService(
            IServiceScopeFactory scopeFactory,
            HealthCheckService health,
            ILogger<MarketplaceParcelBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _health = health;
            _logger = logger;
        }

        public void Wake()
        {
            _wake.Writer.TryWrite(true);
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (_health.IsReady == false && stoppingToken.IsCancellationRequested == false)
            {
                await Task.Delay(TimeSpan.FromSeconds(1), stoppingToken);
            }

            while (!stoppingToken.IsCancellationRequested)
            {
                var full = false;
                try
                {
                    using var scope = _scopeFactory.CreateScope();
                    var dbContext = scope.ServiceProvider.GetRequiredService<Marketplace.Service.DbContext>();
                    var parcelService = scope.ServiceProvider.GetRequiredService<MarketplaceParcelService>();

                    var pending = await dbContext.MarketplaceParcel.GetPendingAsync(BatchSize);
                    var sent = 0;
                    foreach (var parcel in pending)
                    {
                        if (stoppingToken.IsCancellationRequested)
                        {
                            break;
                        }

                        if (await parcelService.SendAsync(parcel))
                        {
                            sent++;
                        }
                    }
                    full = pending.Count >= BatchSize && sent == pending.Count;
                }
                catch (Exception e)
                {
                    _logger.LogError(e, "Error retrying marketplace parcels");
                }

                if (full == false)
                {
                    using var timeout = CancellationTokenSource.CreateLinkedTokenSource(stoppingToken);
                    timeout.CancelAfter(ProcessingInterval);
                    try
                    {
                        await _wake.Reader.ReadAsync(timeout.Token);
                    }
                    catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested == false)
                    {
                    }
                }
            }
        }
    }
}
