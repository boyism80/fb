using Http.Service;
using Marketplace.Model;

namespace Marketplace.Services;

public class MarketplaceParcelService
{
    private readonly StorageService _storageService;
    private readonly Http.Service.DbContext _httpDbContext;
    private readonly Marketplace.Service.DbContext _dbContext;
    private readonly RedisDistributedLockService _distributedLock;
    private readonly ILogger<MarketplaceParcelService> _logger;

    public MarketplaceParcelService(
        StorageService storageService,
        Http.Service.DbContext httpDbContext,
        Marketplace.Service.DbContext dbContext,
        RedisDistributedLockService distributedLock,
        ILogger<MarketplaceParcelService> logger)
    {
        _storageService = storageService;
        _httpDbContext = httpDbContext;
        _dbContext = dbContext;
        _distributedLock = distributedLock;
        _logger = logger;
    }

    // A failed parcel stays pending; MarketplaceParcelBackgroundService retries it.
    public async Task<bool> SendAsync(MarketplaceParcel parcel)
    {
        try
        {
            await using var _ = await _distributedLock.Lock(parcel.World, StorageService.ExternalRefLockKey(parcel.ExternalRef));
            if (await _dbContext.MarketplaceParcel.ExistsAsync(parcel.ExternalRef) &&
                await _httpDbContext.StorageBox.HasLegacyRef(parcel.World, parcel.User, parcel.ExternalRef) == false)
            {
                await _storageService.CreateSystemStorageAsync(
                    world: parcel.World,
                    title: parcel.Title,
                    message: parcel.Message,
                    attachments: parcel.Attachments,
                    externalRef: parcel.ExternalRef,
                    user: parcel.User);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Marketplace parcel {ExternalRef} to user {User} failed", parcel.ExternalRef, parcel.User);
            try
            {
                await _dbContext.MarketplaceParcel.MarkFailedAsync(parcel.ExternalRef, ex.Message);
            }
            catch (Exception markEx)
            {
                _logger.LogError(markEx, "Failed to record marketplace parcel failure {ExternalRef}", parcel.ExternalRef);
            }
            return false;
        }

        try
        {
            await _dbContext.MarketplaceParcel.MarkSentAsync(parcel.ExternalRef);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to mark marketplace parcel {ExternalRef} sent", parcel.ExternalRef);
        }
        return true;
    }
}
