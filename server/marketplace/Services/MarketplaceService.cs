using fb.protocol.marketplace;
using Fb.Model;
using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Marketplace.Extension;
using Marketplace.Model;
using MySqlConnector;

namespace Marketplace.Services;

public class PurchaseItemResult
{
    public MarketplaceListing Listing { get; set; }
    public ushort ActualPurchaseCount { get; set; }
    public ulong RefundAmount { get; set; }
}

public class ListingWithPurchase
{
    public MarketplaceListing Listing { get; set; }
    public MarketplacePurchase Purchase { get; set; }
}

public class MarketplaceService : IMarketplaceService
{
    private readonly MarketplaceDeliveryService _deliveryService;
    private readonly Marketplace.Service.DbContext _dbContext;
    private readonly LogService _logService;

    public MarketplaceService(
        MarketplaceDeliveryService deliveryService,
        Marketplace.Service.DbContext dbContext,
        LogService logService)
    {
        _deliveryService = deliveryService;
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
        ulong price,
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

        // The primary key is the final check: an abort tombstone may land between the pre-check and this insert.
        try
        {
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
        }
        catch (MySqlException e) when (e.ErrorCode == MySqlErrorCode.DuplicateKeyEntry)
        {
            await _logService.WriteAsync("marketplace_list_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "listing_id_already_exists"
            });
            throw new LogicException(ErrorCode.MarketplaceIdAlreadyExists);
        }

        // Log successful listing creation
        await _logService.WriteAsync("marketplace_list_success", new
        {
            character_id = characterId,
            listing_id = listingId
        });

        // Return created listing
        return await _dbContext.Marketplace.GetListingByIdAsync(listingId);
    }

    public async Task<bool> AbortListAsync(
        uint world,
        uint characterId,
        string listingId,
        uint itemModel,
        ushort remainingCount,
        uint? itemDurability,
        string itemCustomName,
        ulong price)
    {
        // A listing that is not found anywhere gets an ABORTED tombstone, so a late list request fails on the primary key.
        for (var attempt = 0; attempt < 2; attempt++)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            await conn.OpenAsync();
            await using var transaction = await conn.BeginTransactionAsync();

            var listing = await _dbContext.Marketplace.GetAnyListingByIdForUpdateAsync(listingId, transaction)
                ?? await _dbContext.Marketplace.GetArchivedListingByIdAsync(listingId, transaction);
            if (listing != null)
            {
                await transaction.CommitAsync();
                if (listing.SellerId != characterId)
                    throw new LogicException(ErrorCode.MarketplaceNotListingOwner);

                var created = listing.Status != ListingState.ABORTED;
                await _logService.WriteAsync("marketplace_abort_list", new
                {
                    character_id = characterId,
                    listing_id = listingId,
                    created = created,
                    status = listing.Status.ToString()
                });
                return created;
            }

            try
            {
                await _dbContext.Marketplace.CreateAbortedListingAsync(
                    listingId,
                    world,
                    characterId,
                    itemModel,
                    remainingCount,
                    itemDurability,
                    itemCustomName,
                    price,
                    transaction);
                await transaction.CommitAsync();
            }
            catch (MySqlException e) when (e.ErrorCode == MySqlErrorCode.DuplicateKeyEntry)
            {
                await transaction.RollbackAsync();
                continue;
            }

            await _logService.WriteAsync("marketplace_abort_list", new
            {
                character_id = characterId,
                listing_id = listingId,
                created = false,
                status = "tombstone"
            });
            return false;
        }

        throw new LogicException(ErrorCode.Unhandled);
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

        await using var conn = _dbContext.GetUnifiedConnection();
        await conn.OpenAsync();
        await using var transaction = await conn.BeginTransactionAsync();

        // The snapshot above only selects the error code; purchase and expire may have changed the row since
        var lockedListing = await _dbContext.Marketplace.GetListingByIdForUpdateAsync(listingId, transaction);
        if (lockedListing == null)
        {
            await transaction.RollbackAsync();
            await _logService.WriteAsync("marketplace_cancel_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "listing_not_found_or_inactive"
            });
            throw new LogicException(ErrorCode.MarketplaceListingNotFound);
        }

        if (await _dbContext.Marketplace.UpdateListingStatusAsync(lockedListing.Id, ListingState.CANCELLED, transaction) == false)
        {
            await transaction.RollbackAsync();
            await _logService.WriteAsync("marketplace_cancel_failed", new
            {
                character_id = characterId,
                listing_id = listingId,
                error = "concurrent_status_change"
            });
            throw new LogicException(ErrorCode.MarketplaceListingNotFound);
        }

        // Return remaining items to seller via storage_box
        var delivery = new MarketplaceDelivery
        {
            ExternalRef = $"marketplace:cancel:{lockedListing.Id}",
            World = world,
            User = lockedListing.SellerId,
            Title = Fb.Model.ConstValue.String.MessageMarketplaceListingCancelledTitle,
            Message = Fb.Model.ConstValue.String.MessageMarketplaceListingCancelledMessage,
            Attachments =
            [
                new Fb.Model.Dsl.Item
                {
                    Id = lockedListing.ItemModel,
                    Count = lockedListing.RemainingCount,
                    Durability = lockedListing.ItemDurability,
                    CustomName = lockedListing.ItemCustomName,
                    Percent = 100.0
                }.ToDSL()
            ]
        };
        await _dbContext.MarketplaceDelivery.CreateAsync(delivery, transaction);

        await transaction.CommitAsync();

        await _deliveryService.DeliverAsync(delivery);

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
        var finished = false;

        try
        {
            // Load listing with row lock to prevent double-purchase
            var listing = await _dbContext.Marketplace.GetListingByIdForUpdateAsync(listingId, transaction);
            if (listing == null)
            {
                await transaction.RollbackAsync();
                finished = true;
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
                finished = true;
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
                finished = true;
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
            var saleDelivery = new MarketplaceDelivery
            {
                ExternalRef = $"marketplace:sale:{purchaseId}",
                World = listing.World,
                User = listing.SellerId,
                Title = Fb.Model.ConstValue.String.MessageMarketplaceSaleTitle,
                Message = string.Format(Fb.Model.ConstValue.String.MessageMarketplaceSaleMessage.ToCSharpFormat(), itemName, actualPurchaseCount, actualPrice),
                Attachments = [new Fb.Model.Dsl.Money { Value = actualPrice }.ToDSL()]
            };
            await _dbContext.MarketplaceDelivery.CreateAsync(saleDelivery, transaction);

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

            var buyDelivery = new MarketplaceDelivery
            {
                ExternalRef = $"marketplace:buy:{purchaseId}",
                World = world,
                User = buyerId,
                Title = buyerTitle,
                Message = buyerMessage,
                Attachments = buyerAttachments
            };
            await _dbContext.MarketplaceDelivery.CreateAsync(buyDelivery, transaction);

            // Commit transaction
            await transaction.CommitAsync();
            finished = true;

            await _deliveryService.DeliverAsync(saleDelivery);
            await _deliveryService.DeliverAsync(buyDelivery);

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
            // Guarded paths already rolled back and logged; after commit the purchase has succeeded.
            if (finished == false)
            {
                await transaction.RollbackAsync();
                await _logService.WriteAsync("marketplace_purchase_failed", new
                {
                    buyer_id = buyerId,
                    listing_id = listingId,
                    purchase_id = purchaseId,
                    error = ex.Message
                });
            }
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
