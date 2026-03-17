using Marketplace.Model;

namespace Marketplace.Services;

public interface IMarketplaceService
{
    Task<MarketplaceListing> ListItemAsync(
        uint world,
        uint characterId,
        string listingId,
        uint itemModel,
        ushort remainingCount,
        uint? itemDurability,
        string itemCustomName,
        uint price,
        TimeSpan expireTime);

    Task CancelListingAsync(uint world, uint characterId, string listingId);

    Task<PurchaseItemResult> PurchaseItemAsync(
        uint world,
        uint buyerId,
        string listingId,
        ushort purchaseCount,
        string purchaseId);

    Task<MarketplaceSearchResult> SearchItemsAsync(MarketplaceSearchOption option);

    Task<MarketplaceListing> GetListingByIdAsync(string listingId);

    Task<List<ListingWithPurchase>> GetListingsByIdsAsync(
        List<string> listingIds,
        uint? buyerId = null);

    Task<Dictionary<string, MarketplacePurchase>> GetPurchasesByIdsAsync(List<string> purchaseIds);
}
