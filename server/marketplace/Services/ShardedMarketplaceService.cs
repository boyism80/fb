using Marketplace.Model;

namespace Marketplace.Services;

public class ShardedMarketplaceService : IMarketplaceService
{
    public Task ListItemAsync(
        uint world,
        uint characterId,
        ulong listingId,
        uint itemModel,
        ushort remainingCount,
        uint? itemDurability,
        string itemCustomName,
        ulong? itemUid,
        ulong price,
        TimeSpan expireTime,
        DateTime deadline)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<bool> AbortListAsync(
        uint world,
        uint characterId,
        ulong listingId,
        uint itemModel,
        ushort remainingCount,
        uint? itemDurability,
        string itemCustomName,
        ulong? itemUid,
        ulong price)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task CancelListingAsync(uint world, uint characterId, ulong listingId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<PurchaseItemResult> PurchaseItemAsync(
        uint world,
        uint buyerId,
        ulong listingId,
        ushort purchaseCount,
        ulong purchaseId,
        DateTime deadline)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<bool> AbortPurchaseAsync(uint world, uint buyerId, ulong listingId, ulong purchaseId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<MarketplaceSearchResult> SearchItemsAsync(MarketplaceSearchOption option)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<MarketplaceListing> GetListingByIdAsync(ulong listingId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<List<ListingWithPurchase>> GetListingsByIdsAsync(
        List<ulong> listingIds,
        uint? buyerId = null)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }
}
