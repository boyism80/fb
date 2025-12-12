using Marketplace.Model;

namespace Marketplace.Services;

/// <summary>
/// Interface for marketplace service functionality.
/// Provides abstraction for single-database and multi-database (sharded) implementations.
/// </summary>
public interface IMarketplaceService
{
    /// <summary>
    /// Allocates a new listing ID (UUID) for a character.
    /// This ID is used to create the actual listing later.
    /// </summary>
    /// <param name="characterId">The character ID requesting the listing ID.</param>
    /// <returns>The allocated listing ID (UUID string).</returns>
    string AllocateListingIdAsync(uint characterId);

    /// <summary>
    /// Creates a new marketplace listing using a pre-allocated listing ID.
    /// </summary>
    /// <param name="listingId">The pre-allocated listing ID (UUID string).</param>
    /// <returns>MarketplaceListing if successful; null if failed.</returns>
    Task<MarketplaceListing> ListItemAsync(
        uint characterId,
        string listingId,
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
    Task CancelListingAsync(uint characterId, string listingId);

    /// <summary>
    /// Purchases an item from a marketplace listing.
    /// </summary>
    /// <returns>MarketplaceListing if successful; null if failed (listing not found or expired).</returns>
    Task<MarketplaceListing> PurchaseItemAsync(
        uint buyerId,
        string listingId);

    /// <summary>
    /// Searches marketplace listings.
    /// </summary>
    /// <returns>MarketplaceSearchResult.</returns>
    Task<MarketplaceSearchResult> SearchItemsAsync(MarketplaceSearchOption option);

    /// <summary>
    /// Checks the status of a listing by listing ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    Task<MarketplaceListing> CheckListingStatusAsync(string listingId);

    /// <summary>
    /// Gets a listing by ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    Task<MarketplaceListing> GetListingByIdAsync(string listingId);
}

