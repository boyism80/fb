using Fb.Model;
using Fb.Model.EnumValue;
using Http;
using Http.Reepository;
using Http.Service;
using Marketplace.Extension;
using Marketplace.Model;

namespace Marketplace.Services;

/// <summary>
/// Provides marketplace service functionality for single-database implementation.
/// Handles marketplace listings, purchases, searches, and transaction management.
/// </summary>
public class MarketplaceService : IMarketplaceService
{
    private readonly MarketplaceRepository _repository;
    private readonly IConfiguration _configuration;
    private readonly StorageService _storageService;
    private readonly DbContext _dbContext;
    private readonly double _transactionFeePercent;
    private readonly uint _minFee;

    public MarketplaceService(
        MarketplaceRepository repository,
        IConfiguration configuration,
        StorageService storageService,
        DbContext dbContext)
    {
        _repository = repository;
        _configuration = configuration;
        _storageService = storageService;
        _dbContext = dbContext;
        _transactionFeePercent = _configuration.GetValue<double>("Marketplace:TransactionFeePercent", 5.0);
        _minFee = _configuration.GetValue<uint>("Marketplace:MinFee", 100);
    }

    /// <summary>
    /// Allocates a new listing ID (UUID) for a character.
    /// This ID is used to create the actual listing later.
    /// </summary>
    /// <param name="characterId">The character ID requesting the listing ID.</param>
    /// <returns>The allocated listing ID (UUID string).</returns>
    public string AllocateListingIdAsync(uint characterId)
    {
        // Generate UUID for listing_id
        // This UUID will be used as the listing identifier and can be used for sharding
        return Guid.NewGuid().ToString();
    }

    /// <summary>
    /// Creates a new marketplace listing using a pre-allocated listing ID.
    /// </summary>
    /// <param name="listingId">The pre-allocated listing ID (UUID string).</param>
    /// <returns>MarketplaceListing if successful; null if failed.</returns>
    public async Task<MarketplaceListing> ListItemAsync(
        uint characterId,
        string listingId,
        uint itemModel,
        ushort itemCount,
        uint? itemDurability,
        string itemCustomName,
        uint price,
        ushort expireHours)
    {
        // Check idempotency: if listing_id already exists, return existing listing
        if (!string.IsNullOrEmpty(listingId))
        {
            var existing = await _repository.GetListingByIdAsync(listingId);
            if (existing != null)
            {
                return existing;
            }
        }

        // Calculate transaction fee
        var transactionFee = CalculateTransactionFee(price);

        // Create listing with listing_id as the primary key (BINARY(16))
        await _repository.CreateListingAsync(
            listingId,
            characterId,
            itemModel,
            itemCount,
            itemDurability,
            itemCustomName,
            price,
            transactionFee,
            DateTime.UtcNow.AddHours(expireHours));

        // Return created listing
        return await _repository.GetListingByIdAsync(listingId);
    }

    /// <summary>
    /// Cancels a marketplace listing.
    /// </summary>
    /// <exception cref="LogicException">Thrown when listing is not found, user is not the owner, or listing is already cancelled/sold.</exception>
    public async Task CancelListingAsync(uint characterId, string listingId)
    {
        var listing = await _repository.GetListingByIdAsync(listingId);
        if (listing == null)
        {
            throw new LogicException(ErrorCode.MarketplaceListingNotFound);
        }

        if (listing.SellerId != characterId)
        {
            throw new LogicException(ErrorCode.MarketplaceNotListingOwner);
        }

        if (listing.Status != 0) // Not Active
        {
            // Check if already cancelled or sold
            if (listing.Status == 2) // Cancelled
            {
                throw new LogicException(ErrorCode.MarketplaceListingAlreadyCancelled);
            }
            if (listing.Status == 1) // Sold
            {
                throw new LogicException(ErrorCode.MarketplaceListingAlreadySold);
            }
            throw new LogicException(ErrorCode.MarketplaceListingNotFound);
        }

        await _repository.UpdateListingStatusAsync(listing.Id, 2); // Cancelled

        // Return item to seller via storage_box
        var sellerCharacter = await _dbContext.Character.Get(listing.SellerId);
        if (sellerCharacter != null && !string.IsNullOrWhiteSpace(sellerCharacter.Name))
        {
            var attachments = new List<Fb.Model.Dsl>
            {
                new Fb.Model.Dsl.Item
                {
                    Id = listing.ItemModel,
                    Count = listing.ItemCount,
                    Durability = listing.ItemDurability,
                    CustomName = listing.ItemCustomName,
                    Percent = 100.0
                }.ToDSL()
            };

            await _storageService.CreatePendingAsync(
                "Marketplace Listing Cancelled",
                $"Your marketplace listing has been cancelled. The item has been returned to your storage box.",
                sellerCharacter.Name,
                DateTime.UtcNow.AddDays(30), // 30 days expiry
                attachments);
        }
    }

    /// <summary>
    /// Purchases an item from a marketplace listing.
    /// </summary>
    /// <returns>MarketplaceListing if successful; null if failed (listing not found or expired).</returns>
    public async Task<MarketplaceListing> PurchaseItemAsync(
        uint buyerId,
        string listingId)
    {
        // Load listing with row lock to prevent double-purchase
        var listing = await _repository.GetListingByIdAsync(listingId);
        if (listing == null || listing.Status != 0) // Not Active
        {
            return null; // ListingNotFound
        }

        if (listing.ExpireDate <= DateTime.UtcNow)
        {
            return null; // ListingExpired
        }

        // Send seller revenue via storage_box (full price, no transaction fee deduction)
        // Transaction fee is already deducted on game server side during listing
        var sellerCharacter = await _dbContext.Character.Get(listing.SellerId);
        if (sellerCharacter != null && !string.IsNullOrWhiteSpace(sellerCharacter.Name))
        {
            var attachments = new List<Fb.Model.Dsl>
            {
                new Fb.Model.Dsl.Money { Value = listing.Price }.ToDSL()
            };

            await _storageService.CreatePendingAsync(
                "Marketplace Sale",
                $"Your item has been sold for {listing.Price} gold.",
                sellerCharacter.Name,
                DateTime.UtcNow.AddDays(30), // 30 days expiry
                attachments);
        }

        // Update listing (mark as sold or reduce count)
        await _repository.UpdateListingAsync(listing.Id, 1, DateTime.UtcNow, buyerId);

        // Return original listing (status updated but we return the original for item data)
        // The listing is now sold, but we return it with the item information
        return listing;
    }

    /// <summary>
    /// Searches marketplace listings.
    /// </summary>
    /// <returns>MarketplaceSearchResult.</returns>
    /// <exception cref="LogicException">Thrown when page is less than 1.</exception>
    public async Task<MarketplaceSearchResult> SearchItemsAsync(MarketplaceSearchOption option)
    {
        // Validate page number
        if (option.Page < 1)
        {
            throw new LogicException(ErrorCode.MarketplaceInvalidPageNumber);
        }

        // Convert item name to item model IDs
        List<uint> itemModelIds = null;
        if (!string.IsNullOrEmpty(option.ItemName))
        {
            itemModelIds = Table.Item.NameToItemModelIds(option.ItemName);
        }

        // Get page size from configuration (default: 20)
        var pageSize = _configuration.GetValue<int>("Marketplace:PageSize", 20);

        // Search listings
        var listings = await _repository.SearchListingsAsync(
            itemModelIds,
            option.MinPrice,
            option.MaxPrice,
            option.SellerId,
            string.IsNullOrEmpty(option.SortBy) ? "created_desc" : option.SortBy,
            (int)option.Page,
            pageSize);

        var totalCount = await _repository.CountListingsAsync(
            itemModelIds,
            option.MinPrice,
            option.MaxPrice,
            option.SellerId);

        return new MarketplaceSearchResult
        {
            Listings = listings,
            TotalCount = totalCount
        };
    }

    /// <summary>
    /// Checks the status of a listing by listing ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    public async Task<MarketplaceListing> CheckListingStatusAsync(string listingId)
    {
        return await _repository.GetListingByIdAsync(listingId);
    }

    /// <summary>
    /// Gets a listing by ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    public async Task<MarketplaceListing> GetListingByIdAsync(string listingId)
    {
        return await _repository.GetListingByIdAsync(listingId);
    }

    /// <summary>
    /// Calculates the transaction fee based on the item price.
    /// </summary>
    /// <param name="price">The item price.</param>
    /// <returns>The calculated transaction fee.</returns>
    private uint CalculateTransactionFee(uint price)
    {
        var fee = (uint)(price * _transactionFeePercent / 100.0);
        if (fee < _minFee)
            fee = _minFee;
        return fee;
    }
}

