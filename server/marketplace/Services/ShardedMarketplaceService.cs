using Marketplace.Model;

namespace Marketplace.Services;

/// <summary>
/// Provides marketplace service functionality for multi-database (sharded) implementation using Elasticsearch.
/// This is a placeholder implementation that will be completed when Elasticsearch integration is ready.
/// </summary>
public class ShardedMarketplaceService : IMarketplaceService
{
    /// <summary>
    /// Allocates a new listing ID (UUID) for a character.
    /// This ID is used to create the actual listing later.
    /// </summary>
    /// <param name="characterId">The character ID requesting the listing ID.</param>
    /// <returns>The allocated listing ID (UUID string).</returns>
    /// <exception cref="NotImplementedException">Thrown until Elasticsearch integration is implemented.</exception>
    public string AllocateListingIdAsync(uint characterId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    /// <summary>
    /// Creates a new marketplace listing using a pre-allocated listing ID.
    /// </summary>
    /// <param name="listingId">The pre-allocated listing ID (UUID string).</param>
    /// <returns>MarketplaceListing if successful; null if failed.</returns>
    /// <exception cref="NotImplementedException">Thrown until Elasticsearch integration is implemented.</exception>
    public Task<MarketplaceListing> ListItemAsync(
        uint characterId,
        string listingId,
        uint itemModel,
        ushort itemCount,
        uint? itemDurability,
        string itemCustomName,
        uint price,
        ushort expireHours)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    /// <summary>
    /// Cancels a marketplace listing.
    /// </summary>
    /// <exception cref="NotImplementedException">Thrown until Elasticsearch integration is implemented.</exception>
    public Task CancelListingAsync(uint characterId, string listingId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    /// <summary>
    /// Purchases an item from a marketplace listing.
    /// </summary>
    /// <returns>MarketplaceListing if successful; null if failed (listing not found or expired).</returns>
    /// <exception cref="NotImplementedException">Thrown until Elasticsearch integration is implemented.</exception>
    public Task<MarketplaceListing> PurchaseItemAsync(
        uint buyerId,
        string listingId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    /// <summary>
    /// Searches marketplace listings.
    /// </summary>
    /// <returns>MarketplaceSearchResult.</returns>
    /// <exception cref="NotImplementedException">Thrown until Elasticsearch integration is implemented.</exception>
    public Task<MarketplaceSearchResult> SearchItemsAsync(MarketplaceSearchOption option)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    /// <summary>
    /// Gets a listing by ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    /// <exception cref="NotImplementedException">Thrown until Elasticsearch integration is implemented.</exception>
    public Task<MarketplaceListing> GetListingByIdAsync(string listingId)
    {
        throw new NotImplementedException("Sharded marketplace service is not yet implemented");
    }

    public Task<List<MarketplaceListing>> GetListingsByIdsAsync(List<string> listingIds)
    {
        throw new NotImplementedException();
    }
}

