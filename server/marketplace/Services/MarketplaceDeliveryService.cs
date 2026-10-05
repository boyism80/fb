using Http.Service;
using Marketplace.Model;

namespace Marketplace.Services;

public class MarketplaceDeliveryService
{
    private readonly StorageService _storageService;
    private readonly Marketplace.Service.DbContext _dbContext;
    private readonly ILogger<MarketplaceDeliveryService> _logger;

    public MarketplaceDeliveryService(
        StorageService storageService,
        Marketplace.Service.DbContext dbContext,
        ILogger<MarketplaceDeliveryService> logger)
    {
        _storageService = storageService;
        _dbContext = dbContext;
        _logger = logger;
    }

    // A failed delivery stays pending; MarketplaceDeliveryBackgroundService retries it.
    public async Task<bool> DeliverAsync(MarketplaceDelivery delivery)
    {
        try
        {
            await _storageService.CreateStorageBoxAsync(
                world: delivery.World,
                userId: delivery.User,
                title: delivery.Title,
                message: delivery.Message,
                attachments: delivery.Attachments,
                externalRef: delivery.ExternalRef);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Marketplace delivery {ExternalRef} to user {User} failed", delivery.ExternalRef, delivery.User);
            try
            {
                await _dbContext.MarketplaceDelivery.MarkFailedAsync(delivery.ExternalRef, ex.Message);
            }
            catch (Exception markEx)
            {
                _logger.LogError(markEx, "Failed to record marketplace delivery failure {ExternalRef}", delivery.ExternalRef);
            }
            return false;
        }

        try
        {
            await _dbContext.MarketplaceDelivery.MarkDeliveredAsync(delivery.ExternalRef);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to mark marketplace delivery {ExternalRef} delivered", delivery.ExternalRef);
        }
        return true;
    }
}
