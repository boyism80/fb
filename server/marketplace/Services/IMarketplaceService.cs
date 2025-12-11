using Marketplace.Model;

namespace Marketplace.Services;

/// <summary>
/// Interface for marketplace service functionality.
/// Provides abstraction for single-database and multi-database (sharded) implementations.
/// </summary>
public interface IMarketplaceService
{
    /// <summary>
    /// Creates a new marketplace listing.
    /// </summary>
    /// <returns>MarketplaceListing if successful; null if failed.</returns>
    Task<MarketplaceListing> ListItemAsync(
        uint characterId,
        string requestId,
        uint itemModel,
        ushort itemCount,
        uint? itemDurability,
        string itemCustomName,
        uint price,
        ushort expireHours);

    /// <summary>
    /// Cancels a marketplace listing.
    /// </summary>
    /// <exception cref="Http.LogicException">Thrown when listing is not found, user is not the owner, or listing is already cancelled/sold.</exception>
    Task CancelListingAsync(uint characterId, ulong listingId);

    /// <summary>
    /// Purchases an item from a marketplace listing.
    /// </summary>
    /// <returns>MarketplaceListing if successful; null if failed (listing not found or expired).</returns>
    Task<MarketplaceListing> PurchaseItemAsync(
        uint buyerId,
        ulong listingId);

    /// <summary>
    /// Searches marketplace listings.
    /// </summary>
    /// <returns>MarketplaceSearchResult.</returns>
    Task<MarketplaceSearchResult> SearchItemsAsync(
        string itemName,
        uint? minPrice,
        uint? maxPrice,
        uint? sellerId,
        string sortBy,
        uint? page,
        uint? pageSize);

    /// <summary>
    /// Checks the status of a listing by request ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    Task<MarketplaceListing> CheckListingStatusAsync(string requestId);

    /// <summary>
    /// Gets a listing by ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    Task<MarketplaceListing> GetListingByIdAsync(ulong listingId);

    /// <summary>
    /// Gets pending transactions or returns for a character.
    /// </summary>
    /// <param name="type">0 = Seller revenue, 1 = Buyer item loss, 2 = Expired returns</param>
    /// <returns>MarketplacePendingResult.</returns>
    Task<MarketplacePendingResult> GetPendingTransactionsAsync(
        uint characterId,
        byte type);

    /// <summary>
    /// Completes a pending transaction by deleting it.
    /// </summary>
    Task<bool> CompletePendingTransactionAsync(ulong transactionId);

    /// <summary>
    /// Completes a pending return by deleting it.
    /// </summary>
    Task<bool> CompletePendingReturnAsync(ulong returnId);
}

