using Marketplace.Model;

namespace Marketplace.Services;

public interface IMarketplaceService
{
    Task ListItemAsync(
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
        DateTime deadline);

    Task<bool> AbortListAsync(
        uint world,
        uint characterId,
        ulong listingId,
        uint itemModel,
        ushort remainingCount,
        uint? itemDurability,
        string itemCustomName,
        ulong? itemUid,
        ulong price);

    Task CancelListingAsync(uint world, uint characterId, ulong listingId);

    Task<PurchaseItemResult> PurchaseItemAsync(
        uint world,
        uint buyerId,
        ulong listingId,
        ushort purchaseCount,
        ulong purchaseId,
        DateTime deadline);

    Task<bool> AbortPurchaseAsync(uint world, uint buyerId, ulong listingId, ulong purchaseId);

    Task<MarketplaceSearchResult> SearchItemsAsync(MarketplaceSearchOption option);

    Task<MarketplaceListing> GetListingByIdAsync(ulong listingId);

    Task<List<ListingWithPurchase>> GetListingsByIdsAsync(
        List<ulong> listingIds,
        uint? buyerId = null);
}
