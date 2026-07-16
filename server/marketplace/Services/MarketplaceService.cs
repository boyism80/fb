using fb.protocol.marketplace;
using Fb.Model;
using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Marketplace.Extension;
using Marketplace.Model;

namespace Marketplace.Services;

public class PurchaseItemResult
{
    public MarketplaceListing Listing { get; set; }
    public ushort ActualPurchaseCount { get; set; }
    public uint RefundAmount { get; set; }
}

public class ListingWithPurchase
{
    public MarketplaceListing Listing { get; set; }
    public MarketplacePurchase Purchase { get; set; }
}

public class MarketplaceService : IMarketplaceService
{
    private readonly StorageService _storageService;
    private readonly Marketplace.Service.DbContext _dbContext;
    private readonly LogService _logService;

    public MarketplaceService(
        StorageService storageService,
        Marketplace.Service.DbContext dbContext,
        LogService logService)
    {
        _storageService = storageService;
        _dbContext = dbContext;
        _logService = logService;
    }

    public async Task<MarketplaceListing> ListItemAsync(
        uint world,
        uint characterId,
        string listingId,
        uint itemModel,
        ushort remainingCount,
        uint? itemDurability,
        string itemCustomName,
        uint price,
        TimeSpan expireTime)
    {
        // Check if listing ID already exists
        if (await _dbContext.Marketplace.CheckListingIdExistsAsync(listingId))
        {
            await _logService.WriteAsync("marketplace_list_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "listing_id_already_exists"
            });
            throw new LogicException(ErrorCode.MarketplaceIdAlreadyExists);
        }

        if (!Table.Item.TryGetValue(itemModel, out var itemDefinition))
        {
            await _logService.WriteAsync("marketplace_list_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                item_model = itemModel,
                error = "item_model_not_found"
            });
            throw new LogicException(ErrorCode.MarketplaceItemNotFound);
        }

        if (!itemDefinition.Trade)
        {
            await _logService.WriteAsync("marketplace_list_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                item_model = itemModel,
                error = "item_not_tradeable"
            });
            throw new LogicException(ErrorCode.MarketplaceItemNotTradeable);
        }

        // Check listing limit for seller
        var activeListingCount = await _dbContext.Marketplace.CountActiveListingsBySellerAsync(characterId);
        if (activeListingCount >= Fb.Model.ConstValue.Marketplace.ListingLimit)
        {
            await _logService.WriteAsync("marketplace_list_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "listing_limit_exceeded",
                active_count = activeListingCount,
                limit = Fb.Model.ConstValue.Marketplace.ListingLimit
            });
            throw new LogicException(ErrorCode.MarketplaceListingLimitExceeded);
        }

        // Log before creating listing
        await _logService.WriteAsync("marketplace_list", new
        {
            character_id = characterId,
            listing_id = listingId,
            item_model = itemModel,
            remaining_count = remainingCount,
            price = price,
            expire_hours = expireTime.ToString()
        });

        // Create listing with listing_id as the primary key
        await _dbContext.Marketplace.CreateListingAsync(
            listingId,
            world,
            characterId,
            itemModel,
            remainingCount,
            itemDurability,
            itemCustomName,
            price,
            DateTime.UtcNow + expireTime);

        // Log successful listing creation
        await _logService.WriteAsync("marketplace_list_success", new
        {
            character_id = characterId,
            listing_id = listingId
        });

        // Return created listing
        return await _dbContext.Marketplace.GetListingByIdAsync(listingId);
    }

    public async Task CancelListingAsync(uint world, uint characterId, string listingId)
    {
        // Log before cancel
        await _logService.WriteAsync("marketplace_cancel", new
        {
            character_id = characterId,
            listing_id = listingId
        });

        var listing = await _dbContext.Marketplace.GetListingByIdAsync(listingId);
        if (listing == null)
        {
            await _logService.WriteAsync("marketplace_cancel_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "listing_not_found"
            });
            throw new LogicException(ErrorCode.MarketplaceListingNotFound);
        }

        if (listing.SellerId != characterId)
        {
            await _logService.WriteAsync("marketplace_cancel_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "not_owner"
            });
            throw new LogicException(ErrorCode.MarketplaceNotListingOwner);
        }

        if (listing.Status != ListingState.ACTIVE)
        {
            // Check if already cancelled or sold
            if (listing.Status == ListingState.CANCELLED)
            {
                await _logService.WriteAsync("marketplace_cancel_failed", new
                {
                    character_id = characterId,
                    listing_id = listingId,
                    error = "already_cancelled"
                });
                throw new LogicException(ErrorCode.MarketplaceListingAlreadyCancelled);
            }
            if (listing.Status == ListingState.SOLD)
            {
                await _logService.WriteAsync("marketplace_cancel_failed", new
                {
                    character_id = characterId,
                    listing_id = listingId,
                    error = "already_sold"
                });
                throw new LogicException(ErrorCode.MarketplaceListingAlreadySold);
            }
            await _logService.WriteAsync("marketplace_cancel_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "invalid_status"
            });
            throw new LogicException(ErrorCode.MarketplaceListingNotFound);
        }

        await _dbContext.Marketplace.UpdateListingStatusAsync(listing.Id, ListingState.CANCELLED);

        // Return remaining items to seller via storage_box
        var attachments = new List<Fb.Model.Dsl>
        {
            new Fb.Model.Dsl.Item
            {
                Id = listing.ItemModel,
                Count = listing.RemainingCount,
                Durability = listing.ItemDurability,
                CustomName = listing.ItemCustomName,
                Percent = 100.0
            }.ToDSL()
        };

        await _storageService.CreateStorageBoxAsync(
            world,
            listing.SellerId,
            Fb.Model.ConstValue.String.MessageMarketplaceListingCancelledTitle,
            Fb.Model.ConstValue.String.MessageMarketplaceListingCancelledMessage,
            attachments: attachments,
            externalRef: $"marketplace:cancel:{listing.Id}");

        // Log successful cancellation
        await _logService.WriteAsync("marketplace_cancel_success", new
        {
            character_id = characterId,
            listing_id = listingId
        });
    }

    public async Task<PurchaseItemResult> PurchaseItemAsync(
        uint world,
        uint buyerId,
        string listingId,
        ushort purchaseCount,
        string purchaseId)
    {
        // Check if purchase ID already exists
        if (await _dbContext.MarketplacePurchase.CheckPurchaseIdExistsAsync(purchaseId))
        {
            await _logService.WriteAsync("marketplace_purchase_failed", new
            {
                buyer_id = buyerId,
                listing_id = listingId,
                purchase_id = purchaseId,
                error = "purchase_id_already_exists"
            });
            throw new LogicException(ErrorCode.MarketplaceIdAlreadyExists);
        }

        // Log before purchase
        await _logService.WriteAsync("marketplace_purchase", new
        {
            buyer_id = buyerId,
            listing_id = listingId,
            purchase_id = purchaseId,
            purchase_count = purchaseCount
        });

        // Use transaction to ensure atomicity
        await using var conn = _dbContext.GetUnifiedConnection();
        await conn.OpenAsync();
        await using var transaction = await conn.BeginTransactionAsync();

        try
        {
            // Load listing with row lock to prevent double-purchase
            var listing = await _dbContext.Marketplace.GetListingByIdForUpdateAsync(listingId, transaction);
            if (listing == null)
            {
                await transaction.RollbackAsync();
                await _logService.WriteAsync("marketplace_purchase_failed", new
                {
                    buyer_id = buyerId,
                    listing_id = listingId,
                    purchase_id = purchaseId,
                    error = "listing_not_found_or_inactive"
                });
                throw new LogicException(ErrorCode.MarketplaceListingNotFound);
            }

            // Determine actual purchase count (may be less than requested if insufficient stock)
            var actualPurchaseCount = (ushort)Math.Min(purchaseCount, listing.RemainingCount);
            if (actualPurchaseCount == 0)
            {
                await transaction.RollbackAsync();
                await _logService.WriteAsync("marketplace_purchase_failed", new
                {
                    buyer_id = buyerId,
                    listing_id = listingId,
                    purchase_id = purchaseId,
                    error = "insufficient_stock"
                });
                throw new LogicException(ErrorCode.MarketplaceInsufficientStock);
            }

            // Calculate prices
            var expectedPrice = listing.Price * purchaseCount;
            var actualPrice = listing.Price * actualPurchaseCount;
            var refundAmount = expectedPrice - actualPrice;

            // Update listing (decrement remaining_count, update status/sold_date if needed)
            var (updateSuccess, newRemainingCount, isSold) = await _dbContext.Marketplace.UpdateListingAsync(
                listingId,
                actualPurchaseCount,
                transaction);

            if (!updateSuccess)
            {
                // Another transaction already purchased or insufficient stock
                await transaction.RollbackAsync();
                await _logService.WriteAsync("marketplace_purchase_failed", new
                {
                    buyer_id = buyerId,
                    listing_id = listingId,
                    purchase_id = purchaseId,
                    error = "concurrent_purchase_conflict_or_insufficient_stock"
                });
                throw new LogicException(ErrorCode.MarketplaceInsufficientStock);
            }

            // Create purchase record
            await _dbContext.MarketplacePurchase.CreatePurchaseAsync(
                purchaseId,
                world,
                listingId,
                buyerId,
                actualPurchaseCount,
                actualPrice,
                transaction);

            // Get item name for sale message
            var itemName = "unknown item name";
            if (Table.Item.TryGetValue(listing.ItemModel, out var model))
            {
                var itemModel = Table.Item[listing.ItemModel];
                itemName = itemModel.Name;
            }

            // Send seller revenue via storage_box (full price, no fees deducted)
            await _storageService.CreateStorageBoxAsync(
                listing.World,
                listing.SellerId,
                Fb.Model.ConstValue.String.MessageMarketplaceSaleTitle,
                string.Format(Fb.Model.ConstValue.String.MessageMarketplaceSaleMessage.ToCSharpFormat(), itemName, actualPurchaseCount, actualPrice),
                attachments: [new Fb.Model.Dsl.Money { Value = actualPrice }.ToDSL()],
                externalRef: $"marketplace:sale:{purchaseId}");

            // Prepare buyer attachments (item + refund if any) - all in one storage box
            var buyerAttachments = new List<Fb.Model.Dsl>
            {
                new Fb.Model.Dsl.Item
                {
                    Id = listing.ItemModel,
                    Count = actualPurchaseCount,
                    Durability = listing.ItemDurability,
                    CustomName = listing.ItemCustomName,
                    Percent = 100.0
                }.ToDSL()
            };

            // Add refund amount if there's a discrepancy
            if (refundAmount > 0)
            {
                buyerAttachments.Add(new Fb.Model.Dsl.Money { Value = refundAmount }.ToDSL());
            }

            // Get item name for purchase message (reuse from sale message)
            // Send purchased item and refund (if any) to buyer via storage_box
            // Use different message if refund is included
            string buyerTitle;
            string buyerMessage;
            if (refundAmount > 0)
            {
                buyerTitle = Fb.Model.ConstValue.String.MessageMarketplacePurchaseRefundTitle;
                buyerMessage = string.Format(Fb.Model.ConstValue.String.MessageMarketplacePurchaseRefundMessage.ToCSharpFormat(), itemName, actualPurchaseCount, actualPrice, refundAmount);
            }
            else
            {
                buyerTitle = Fb.Model.ConstValue.String.MessageMarketplacePurchaseTitle;
                buyerMessage = string.Format(Fb.Model.ConstValue.String.MessageMarketplacePurchaseMessage.ToCSharpFormat(), itemName, actualPurchaseCount, actualPrice);
            }

            await _storageService.CreateStorageBoxAsync(
                world,
                buyerId,
                buyerTitle,
                buyerMessage,
                attachments: buyerAttachments,
                externalRef: $"marketplace:buy:{purchaseId}");

            // Commit transaction
            await transaction.CommitAsync();

            // Log successful purchase
            await _logService.WriteAsync("marketplace_purchase_success", new
            {
                buyer_id = buyerId,
                listing_id = listingId,
                purchase_id = purchaseId,
                seller_id = listing.SellerId,
                expected_count = purchaseCount,
                actual_count = actualPurchaseCount,
                expected_price = expectedPrice,
                actual_price = actualPrice,
                refund_amount = refundAmount
            });

            // Return listing with purchase information
            return new PurchaseItemResult
            {
                Listing = listing,
                ActualPurchaseCount = actualPurchaseCount,
                RefundAmount = refundAmount
            };
        }
        catch (Exception ex)
        {
            await transaction.RollbackAsync();
            await _logService.WriteAsync("marketplace_purchase_failed", new
            {
                buyer_id = buyerId,
                listing_id = listingId,
                purchase_id = purchaseId,
                error = ex.Message
            });
            throw;
        }
    }

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

        // Use transaction to ensure atomicity between search and count
        await using var conn = _dbContext.GetUnifiedConnection();
        await conn.OpenAsync();
        await using var transaction = await conn.BeginTransactionAsync();

        try
        {
            // Search listings and count within the same transaction
            var listings = await _dbContext.Marketplace.SearchListingsAsync(
                itemModelIds,
                option.MinPrice,
                option.MaxPrice,
                option.SellerId,
                string.IsNullOrEmpty(option.SortBy) ? "name_price_asc" : option.SortBy,
                (int)option.Page,
                Fb.Model.ConstValue.Marketplace.PageSize,
                transaction);

            var totalCount = await _dbContext.Marketplace.CountListingsAsync(
                itemModelIds,
                option.MinPrice,
                option.MaxPrice,
                option.SellerId,
                transaction);

            await transaction.CommitAsync();

            return new MarketplaceSearchResult
            {
                Listings = listings,
                TotalCount = totalCount
            };
        }
        catch
        {
            await transaction.RollbackAsync();
            throw;
        }
    }

    public async Task<MarketplaceListing> GetListingByIdAsync(string listingId)
    {
        return await _dbContext.Marketplace.GetListingByIdAsync(listingId);
    }

    public async Task<List<ListingWithPurchase>> GetListingsByIdsAsync(
        List<string> listingIds,
        uint? buyerId = null)
    {
        var listings = await _dbContext.Marketplace.GetListingsByIdsAsync(listingIds);

        if (!buyerId.HasValue || listings.Count == 0)
        {
            return listings.Select(l => new ListingWithPurchase
            {
                Listing = l,
                Purchase = null
            }).ToList();
        }

        // Get purchase records for the buyer
        var purchases = await _dbContext.Marketplace.GetPurchasesByListingIdsAndBuyerAsync(listingIds, buyerId.Value);

        return listings.Select(l =>
        {
            purchases.TryGetValue(l.Id, out var purchase);
            return new ListingWithPurchase
            {
                Listing = l,
                Purchase = purchase
            };
        }).ToList();
    }

    public async Task<Dictionary<string, MarketplacePurchase>> GetPurchasesByIdsAsync(List<string> purchaseIds)
    {
        return await _dbContext.MarketplacePurchase.GetPurchasesByIdsAsync(purchaseIds);
    }

}
