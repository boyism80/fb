using Fb.Model;
using Fb.Model.EnumValue;
using Http;
using Http.Reepository;
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
    private readonly double _listingFeePercent;
    private readonly double _transactionFeePercent;
    private readonly uint _minFee;

    public MarketplaceService(
        MarketplaceRepository repository,
        IConfiguration configuration)
    {
        _repository = repository;
        _configuration = configuration;
        _listingFeePercent = _configuration.GetValue<double>("Marketplace:ListingFeePercent", 5.0);
        _transactionFeePercent = _configuration.GetValue<double>("Marketplace:TransactionFeePercent", 5.0);
        _minFee = _configuration.GetValue<uint>("Marketplace:MinFee", 100);
    }

    /// <summary>
    /// Creates a new marketplace listing.
    /// </summary>
    /// <returns>MarketplaceListing if successful; null if failed.</returns>
    public async Task<MarketplaceListing> ListItemAsync(
        uint characterId,
        string requestId,
        uint itemModel,
        ushort itemCount,
        uint? itemDurability,
        string itemCustomName,
        uint price,
        ushort expireHours)
    {
        // Check idempotency: if request_id already exists, return existing listing
        if (!string.IsNullOrEmpty(requestId))
        {
            var existing = await _repository.GetListingByRequestIdAsync(requestId);
            if (existing != null)
            {
                return existing;
            }
        }

        // Calculate fees
        var listingFee = CalculateListingFee(price);
        var transactionFee = CalculateTransactionFee(price);

        // Create listing
        var listingId = await _repository.CreateListingAsync(
            characterId,
            requestId,
            itemModel,
            itemCount,
            itemDurability,
            itemCustomName,
            price,
            listingFee,
            transactionFee,
            DateTime.UtcNow.AddHours(expireHours));

        // Return created listing
        return await _repository.GetListingByIdAsync(listingId);
    }

    /// <summary>
    /// Cancels a marketplace listing.
    /// </summary>
    /// <exception cref="LogicException">Thrown when listing is not found, user is not the owner, or listing is already cancelled/sold.</exception>
    public async Task CancelListingAsync(uint characterId, ulong listingId)
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
    }

    /// <summary>
    /// Purchases an item from a marketplace listing.
    /// </summary>
    /// <returns>MarketplaceListing if successful; null if failed (listing not found or expired).</returns>
    public async Task<MarketplaceListing> PurchaseItemAsync(
        uint buyerId,
        ulong listingId)
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

        // Calculate fees and revenue
        var transactionFee = CalculateTransactionFee(listing.Price);
        var sellerRevenue = listing.Price - transactionFee;

        // Update listing (mark as sold or reduce count)
        await _repository.UpdateListingAsync(listing.Id, 1, DateTime.UtcNow, buyerId);

        // Create transaction record
        await _repository.CreateTransactionAsync(
            listing.Id,
            listing.SellerId,
            buyerId,
            listing.ItemModel,
            listing.ItemCount,
            listing.Price,
            listing.ListingFee,
            transactionFee,
            sellerRevenue);

        // Create pending transaction for seller revenue (if seller is on different server)
        // This will be handled by game server reconciliation
        await _repository.CreatePendingTransactionAsync(
            listing.Id,
            buyerId,
            listing.SellerId,
            listing.ItemModel,
            listing.ItemCount,
            listing.Price,
            sellerRevenue);

        // Return original listing (status updated but we return the original for item data)
        // The listing is now sold, but we return it with the item information
        return listing;
    }

    /// <summary>
    /// Searches marketplace listings.
    /// </summary>
    /// <returns>MarketplaceSearchResult.</returns>
    public async Task<MarketplaceSearchResult> SearchItemsAsync(
        string itemName,
        uint? minPrice,
        uint? maxPrice,
        uint? sellerId,
        string sortBy,
        uint? page,
        uint? pageSize)
    {
        // Convert item name to item model IDs
        List<uint> itemModelIds = null;
        if (!string.IsNullOrEmpty(itemName))
        {
            itemModelIds = Table.Item.NameToItemModelIds(itemName);
        }

        // Search listings
        var listings = await _repository.SearchListingsAsync(
            itemModelIds,
            minPrice,
            maxPrice,
            sellerId,
            string.IsNullOrEmpty(sortBy) ? "created_desc" : sortBy,
            (int)(page ?? 1),
            (int)(pageSize ?? 20));

        var totalCount = await _repository.CountListingsAsync(
            itemModelIds,
            minPrice,
            maxPrice,
            sellerId);

        return new MarketplaceSearchResult
        {
            Listings = listings,
            TotalCount = totalCount
        };
    }

    /// <summary>
    /// Checks the status of a listing by request ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    public async Task<MarketplaceListing> CheckListingStatusAsync(string requestId)
    {
        return await _repository.GetListingByRequestIdAsync(requestId);
    }

    /// <summary>
    /// Gets a listing by ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    public async Task<MarketplaceListing> GetListingByIdAsync(ulong listingId)
    {
        return await _repository.GetListingByIdAsync(listingId);
    }

    /// <summary>
    /// Gets pending transactions or returns for a character.
    /// </summary>
    /// <param name="type">0 = Seller revenue, 1 = Buyer item loss, 2 = Expired returns</param>
    /// <returns>MarketplacePendingResult.</returns>
    public async Task<MarketplacePendingResult> GetPendingTransactionsAsync(
        uint characterId,
        byte type)
    {
        List<PendingTransaction> transactions = null;
        List<PendingReturn> returns = null;

        switch (type)
        {
            case 0: // Seller revenue
                transactions = await _repository.GetPendingTransactionsBySellerIdAsync(characterId);
                break;
            case 1: // Buyer item loss
                transactions = await _repository.GetPendingTransactionsByBuyerIdAsync(characterId);
                break;
            case 2: // Expired returns
                returns = await _repository.GetPendingReturnsBySellerIdAsync(characterId);
                break;
        }

        return new MarketplacePendingResult
        {
            Transactions = transactions,
            Returns = returns
        };
    }

    /// <summary>
    /// Completes a pending transaction by deleting it.
    /// </summary>
    public async Task<bool> CompletePendingTransactionAsync(ulong transactionId)
    {
        return await _repository.DeletePendingTransactionAsync(transactionId);
    }

    /// <summary>
    /// Completes a pending return by deleting it.
    /// </summary>
    public async Task<bool> CompletePendingReturnAsync(ulong returnId)
    {
        return await _repository.DeletePendingReturnAsync(returnId);
    }

    /// <summary>
    /// Calculates the listing fee based on the item price.
    /// </summary>
    /// <param name="price">The item price.</param>
    /// <returns>The calculated listing fee.</returns>
    private uint CalculateListingFee(uint price)
    {
        var fee = (uint)(price * _listingFeePercent / 100.0);
        if (fee < _minFee)
            fee = _minFee;
        return fee;
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

