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
    private readonly LogService _logService;
    private readonly double _transactionFeePercent;
    private readonly uint _minFee;

    public MarketplaceService(
        MarketplaceRepository repository,
        IConfiguration configuration,
        StorageService storageService,
        DbContext dbContext,
        LogService logService)
    {
        _repository = repository;
        _configuration = configuration;
        _storageService = storageService;
        _dbContext = dbContext;
        _logService = logService;
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
        // Log before creating listing
        _logService?.Write("marketplace_list", new
        {
            character_id = characterId,
            listing_id = listingId,
            item_model = itemModel,
            item_count = itemCount,
            price = price,
            expire_hours = expireHours
        });

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

        // Log successful listing creation
        _logService?.Write("marketplace_list_success", new
        {
            character_id = characterId,
            listing_id = listingId
        });

        // Return created listing
        return await _repository.GetListingByIdAsync(listingId);
    }

    /// <summary>
    /// Cancels a marketplace listing.
    /// </summary>
    /// <exception cref="LogicException">Thrown when listing is not found, user is not the owner, or listing is already cancelled/sold.</exception>
    public async Task CancelListingAsync(uint characterId, string listingId)
    {
        // Log before cancel
        _logService?.Write("marketplace_cancel", new
        {
            character_id = characterId,
            listing_id = listingId
        });

        var listing = await _repository.GetListingByIdAsync(listingId);
        if (listing == null)
        {
            _logService?.Write("marketplace_cancel_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "listing_not_found"
            });
            throw new LogicException(ErrorCode.MarketplaceListingNotFound);
        }

        if (listing.SellerId != characterId)
        {
            _logService?.Write("marketplace_cancel_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "not_owner"
            });
            throw new LogicException(ErrorCode.MarketplaceNotListingOwner);
        }

        if (listing.Status != 0) // Not Active
        {
            // Check if already cancelled or sold
            if (listing.Status == 2) // Cancelled
            {
                _logService?.Write("marketplace_cancel_failed", new
                {
                    character_id = characterId,
                    listing_id = listingId,
                    error = "already_cancelled"
                });
                throw new LogicException(ErrorCode.MarketplaceListingAlreadyCancelled);
            }
            if (listing.Status == 1) // Sold
            {
                _logService?.Write("marketplace_cancel_failed", new
                {
                    character_id = characterId,
                    listing_id = listingId,
                    error = "already_sold"
                });
                throw new LogicException(ErrorCode.MarketplaceListingAlreadySold);
            }
            _logService?.Write("marketplace_cancel_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "invalid_status"
            });
            throw new LogicException(ErrorCode.MarketplaceListingNotFound);
        }

        await _repository.UpdateListingStatusAsync(listing.Id, 2); // Cancelled

        // Return item to seller via storage_box
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
            Fb.Model.ConstValue.String.MessageMarketplaceListingCancelledTitle,
            Fb.Model.ConstValue.String.MessageMarketplaceListingCancelledMessage,
            listing.SellerId,
            DateTime.UtcNow.AddDays(30), // 30 days expiry
            attachments);

        // Log successful cancellation
        _logService?.Write("marketplace_cancel_success", new
        {
            character_id = characterId,
            listing_id = listingId
        });
    }

    /// <summary>
    /// Purchases an item from a marketplace listing.
    /// Uses database transaction and row locking to prevent concurrent purchases.
    /// </summary>
    /// <returns>MarketplaceListing if successful; null if failed (listing not found or expired).</returns>
    public async Task<MarketplaceListing> PurchaseItemAsync(
        uint buyerId,
        string listingId)
    {
        // Log before purchase
        _logService?.Write("marketplace_purchase", new
        {
            buyer_id = buyerId,
            listing_id = listingId
        });

        // Use transaction to ensure atomicity
        await using var conn = _dbContext.Connection(-1);
        await conn.OpenAsync();
        await using var transaction = await conn.BeginTransactionAsync();

        try
        {
            // Load listing with row lock to prevent double-purchase
            var listing = await _repository.GetListingByIdForUpdateAsync(listingId, transaction);
            if (listing == null)
            {
                await transaction.RollbackAsync();
                _logService?.Write("marketplace_purchase_failed", new
                {
                    buyer_id = buyerId,
                    listing_id = listingId,
                    error = "listing_not_found_or_inactive"
                });
                return null; // ListingNotFound or not Active or expired
            }

            // Send seller revenue via storage_box (full price, no transaction fee deduction)
            // Transaction fee is already deducted on game server side during listing
            await _storageService.CreatePendingAsync(
                Fb.Model.ConstValue.String.MessageMarketplaceSaleTitle,
                string.Format(Fb.Model.ConstValue.String.MessageMarketplaceSaleMessage.ToCSharpFormat(), listing.Price),
                listing.SellerId,
                DateTime.UtcNow.AddDays(30), // 30 days expiry
                [
                    new Fb.Model.Dsl.Money { Value = listing.Price }.ToDSL()
                ]);

            // Send purchased item to buyer via storage_box
            await _storageService.CreatePendingAsync(
                Fb.Model.ConstValue.String.MessageMarketplacePurchaseTitle,
                Fb.Model.ConstValue.String.MessageMarketplacePurchaseMessage,
                buyerId,
                DateTime.UtcNow.AddDays(30), // 30 days expiry
                [
                    new Fb.Model.Dsl.Item
                    {
                        Id = listing.ItemModel,
                        Count = listing.ItemCount,
                        Durability = listing.ItemDurability,
                        CustomName = listing.ItemCustomName,
                        Percent = 100.0
                    }.ToDSL()
                ]);

            // Update listing (mark as sold or reduce count) with status check
            var updated = await _repository.UpdateListingAsync(listing.Id, 1, DateTime.UtcNow, buyerId, transaction);
            if (!updated)
            {
                // Another transaction already purchased this listing
                await transaction.RollbackAsync();
                _logService?.Write("marketplace_purchase_failed", new
                {
                    buyer_id = buyerId,
                    listing_id = listingId,
                    error = "concurrent_purchase_conflict"
                });
                return null;
            }

            // Commit transaction
            await transaction.CommitAsync();

            // Log successful purchase
            _logService?.Write("marketplace_purchase_success", new
            {
                buyer_id = buyerId,
                listing_id = listingId,
                seller_id = listing.SellerId,
                price = listing.Price
            });

            // Return original listing (status updated but we return the original for item data)
            // The listing is now sold, but we return it with the item information
            return listing;
        }
        catch (Exception ex)
        {
            await transaction.RollbackAsync();
            _logService?.Write("marketplace_purchase_failed", new
            {
                buyer_id = buyerId,
                listing_id = listingId,
                error = ex.Message
            });
            throw;
        }
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
            string.IsNullOrEmpty(option.SortBy) ? "name_price_asc" : option.SortBy,
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
    /// Gets a listing by ID.
    /// </summary>
    /// <returns>MarketplaceListing if found; null if not found.</returns>
    public async Task<MarketplaceListing> GetListingByIdAsync(string listingId)
    {
        return await _repository.GetListingByIdAsync(listingId);
    }

    /// <summary>
    /// Gets multiple listings by their IDs.
    /// </summary>
    /// <param name="listingIds">List of listing IDs to retrieve.</param>
    /// <returns>List of marketplace listings found.</returns>
    public async Task<List<MarketplaceListing>> GetListingsByIdsAsync(List<string> listingIds)
    {
        return await _repository.GetListingsByIdsAsync(listingIds);
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

