using Marketplace.Model;

namespace Marketplace.Services;

public class ShardedMarketplaceService : IMarketplaceService
{
    public Task ListItemAsync(
        uint world,
        uint characterId,
        string listingId,
        uint itemModel,
        ushort remainingCount,
        uint? itemDurability,
        string itemCustomName,
        ulong price,
        TimeSpan expireTime,
        DateTime deadline)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<bool> AbortListAsync(
        uint world,
        uint characterId,
        string listingId,
        uint itemModel,
        ushort remainingCount,
        uint? itemDurability,
        string itemCustomName,
        ulong price)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task CancelListingAsync(uint world, uint characterId, string listingId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<PurchaseItemResult> PurchaseItemAsync(
        uint world,
        uint buyerId,
        string listingId,
        ushort purchaseCount,
        string purchaseId,
        DateTime deadline)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<bool> AbortPurchaseAsync(uint world, uint buyerId, string listingId, string purchaseId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<MarketplaceSearchResult> SearchItemsAsync(MarketplaceSearchOption option)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<MarketplaceListing> GetListingByIdAsync(string listingId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<List<ListingWithPurchase>> GetListingsByIdsAsync(
        List<string> listingIds,
        uint? buyerId = null)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }
}
