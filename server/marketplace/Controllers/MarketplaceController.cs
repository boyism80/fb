using Fb.Model.EnumValue;
using Http;
using Marketplace.Services;
using Microsoft.AspNetCore.Mvc;
using Protocol = fb.protocol.marketplace;
using Request = fb.protocol.marketplace.request;
using Response = fb.protocol.marketplace.response;

namespace Marketplace.Controllers;

[ApiController]
[Route("marketplace")]
public class MarketplaceController : ControllerBase
{
    private readonly ILogger<MarketplaceController> _logger;
    private readonly IMarketplaceService _marketplaceService;

    public MarketplaceController(
        ILogger<MarketplaceController> logger,
        IMarketplaceService marketplaceService)
    {
        _logger = logger;
        _marketplaceService = marketplaceService;
    }

    [HttpPost("list")]
    public async Task<Response.MarketplaceList> ListItem(Request.MarketplaceList request)
    {
        try
        {
            var listing = await _marketplaceService.ListItemAsync(
                request.CharacterId,
                request.RequestId ?? string.Empty,
                request.Item.Model,
                request.Item.Count,
                request.Item.Durability,
                request.Item.CustomName ?? string.Empty,
                request.Price,
                request.ExpireHours) ?? throw new LogicException(ErrorCode.Unhandled);

            return new Response.MarketplaceList
            {
                Success = true,
                ListingId = listing.Id,
                ListingFee = listing.ListingFee,
                RequestId = listing.RequestId ?? string.Empty,
                Error = (uint)ErrorCode.None
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create listing for character {CharacterId}", request.CharacterId);
            return new Response.MarketplaceList
            {
                Success = false,
                ListingId = 0,
                ListingFee = 0,
                RequestId = request.RequestId ?? string.Empty,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("cancel")]
    public async Task<Response.MarketplaceCancel> CancelListing(Request.MarketplaceCancel request)
    {
        try
        {
            await _marketplaceService.CancelListingAsync(request.CharacterId, request.ListingId);

            return new Response.MarketplaceCancel
            {
                Success = true,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            return new Response.MarketplaceCancel
            {
                Success = false,
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to cancel listing {ListingId}", request.ListingId);
            return new Response.MarketplaceCancel
            {
                Success = false,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("purchase")]
    public async Task<Response.MarketplacePurchase> PurchaseItem(Request.MarketplacePurchase request)
    {
        try
        {
            var listing = await _marketplaceService.PurchaseItemAsync(request.BuyerId, request.ListingId);

            if (listing != null)
            {
                return new Response.MarketplacePurchase
                {
                    Success = true,
                    Item = new Protocol.MarketplaceItem
                    {
                        Owner = request.BuyerId,
                        Model = listing.ItemModel,
                        Count = listing.ItemCount,
                        Durability = listing.ItemDurability,
                        CustomName = listing.ItemCustomName ?? string.Empty
                    },
                    Error = (uint)ErrorCode.None
                };
            }

            // Try to get listing to determine specific error
            var listingCheck = await _marketplaceService.GetListingByIdAsync(request.ListingId);
            uint errorCode = (uint)ErrorCode.MarketplaceListingNotFound;
            if (listingCheck != null)
            {
                if (listingCheck.ExpireDate <= DateTime.UtcNow)
                {
                    errorCode = (uint)ErrorCode.MarketplaceListingExpired;
                }
                else if (listingCheck.Status == 1)
                {
                    errorCode = (uint)ErrorCode.MarketplaceListingAlreadySold;
                }
                else if (listingCheck.Status == 2)
                {
                    errorCode = (uint)ErrorCode.MarketplaceListingAlreadyCancelled;
                }
            }

            return new Response.MarketplacePurchase
            {
                Success = false,
                Item = new Protocol.MarketplaceItem
                {
                    Owner = 0,
                    Model = 0,
                    Count = 0,
                    Durability = null,
                    CustomName = string.Empty
                },
                Error = errorCode
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to purchase listing {ListingId}", request.ListingId);
            return new Response.MarketplacePurchase
            {
                Success = false,
                Item = new Protocol.MarketplaceItem
                {
                    Owner = 0,
                    Model = 0,
                    Count = 0,
                    Durability = null,
                    CustomName = string.Empty
                },
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("search")]
    public async Task<Response.MarketplaceSearch> SearchItems(Request.MarketplaceSearch request)
    {
        try
        {
            var result = await _marketplaceService.SearchItemsAsync(
                request.ItemName ?? string.Empty,
                request.MinPrice,
                request.MaxPrice,
                request.SellerId,
                request.SortBy ?? string.Empty,
                request.Page,
                request.PageSize);

            var protocolListings = result.Listings.Select(l => new Protocol.MarketplaceListing
            {
                Id = l.Id,
                SellerId = l.SellerId,
                Item = new Protocol.MarketplaceItem
                {
                    Owner = l.SellerId,
                    Model = l.ItemModel,
                    Count = l.ItemCount,
                    Durability = l.ItemDurability,
                    CustomName = l.ItemCustomName ?? string.Empty
                },
                Price = l.Price,
                ListingFee = l.ListingFee,
                TransactionFee = l.TransactionFee,
                ExpireDate = l.ExpireDate.ToString("yyyy-MM-dd HH:mm:ss"),
                CreatedDate = l.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")
            }).ToList();

            var searchResult = new Protocol.MarketplaceSearchResult
            {
                Listings = protocolListings,
                TotalCount = (uint)result.TotalCount,
                Page = request.Page ?? 1,
                PageSize = request.PageSize ?? 20
            };

            return new Response.MarketplaceSearch
            {
                Success = true,
                Result = searchResult,
                Error = (uint)ErrorCode.None
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to search listings");
            return new Response.MarketplaceSearch
            {
                Success = false,
                Result = new Protocol.MarketplaceSearchResult
                {
                    Listings = new List<Protocol.MarketplaceListing>(),
                    TotalCount = 0,
                    Page = request.Page ?? 1,
                    PageSize = request.PageSize ?? 20
                },
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("check-status")]
    public async Task<Response.MarketplaceCheckListingStatus> CheckListingStatus(Request.MarketplaceCheckListingStatus request)
    {
        try
        {
            var listing = await _marketplaceService.CheckListingStatusAsync(request.RequestId);

            if (listing != null)
            {
                return new Response.MarketplaceCheckListingStatus
                {
                    Success = true,
                    ListingId = listing.Id,
                    ListingFee = listing.ListingFee,
                    Error = (uint)ErrorCode.None
                };
            }

            return new Response.MarketplaceCheckListingStatus
            {
                Success = true,
                ListingId = 0,
                ListingFee = 0,
                Error = (uint)ErrorCode.None
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to check listing status for request {RequestId}", request.RequestId);
            return new Response.MarketplaceCheckListingStatus
            {
                Success = false,
                ListingId = 0,
                ListingFee = 0,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("pending")]
    public async Task<Response.MarketplaceGetPendingTransactions> GetPendingTransactions(Request.MarketplaceGetPendingTransactions request)
    {
        try
        {
            var result = await _marketplaceService.GetPendingTransactionsAsync(request.CharacterId, request.Type);

            var protocolTransactions = (result.Transactions != null ? result.Transactions : new List<Marketplace.Model.PendingTransaction>()).Select(t => new Protocol.PendingTransaction
            {
                Id = t.Id,
                ListingId = t.ListingId,
                BuyerId = t.BuyerId,
                SellerId = t.SellerId,
                ItemModel = t.ItemModel,
                ItemCount = t.ItemCount,
                Price = t.Price,
                SellerRevenue = t.SellerRevenue,
                CreatedDate = t.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")
            }).ToList();

            var protocolReturns = (result.Returns != null ? result.Returns : new List<Marketplace.Model.PendingReturn>()).Select(r => new Protocol.PendingReturn
            {
                Id = r.Id,
                ListingId = r.ListingId,
                SellerId = r.SellerId,
                ItemModel = r.ItemModel,
                ItemCount = r.ItemCount,
                ItemDurability = r.ItemDurability,
                ItemCustomName = r.ItemCustomName ?? string.Empty,
                CreatedDate = r.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")
            }).ToList();

            return new Response.MarketplaceGetPendingTransactions
            {
                Success = true,
                Transactions = protocolTransactions,
                Returns = protocolReturns,
                Error = (uint)ErrorCode.None
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to get pending transactions for character {CharacterId}", request.CharacterId);
            return new Response.MarketplaceGetPendingTransactions
            {
                Success = false,
                Transactions = new List<Protocol.PendingTransaction>(),
                Returns = new List<Protocol.PendingReturn>(),
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("complete-transaction")]
    public async Task<Response.MarketplaceCompletePendingTransaction> CompletePendingTransaction(Request.MarketplaceCompletePendingTransaction request)
    {
        try
        {
            var success = await _marketplaceService.CompletePendingTransactionAsync(request.TransactionId);

            return new Response.MarketplaceCompletePendingTransaction
            {
                Success = success
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to complete pending transaction {TransactionId}", request.TransactionId);
            return new Response.MarketplaceCompletePendingTransaction
            {
                Success = false
            };
        }
    }

    [HttpPost("complete-return")]
    public async Task<Response.MarketplaceCompletePendingReturn> CompletePendingReturn(Request.MarketplaceCompletePendingReturn request)
    {
        try
        {
            var success = await _marketplaceService.CompletePendingReturnAsync(request.ReturnId);

            return new Response.MarketplaceCompletePendingReturn
            {
                Success = success
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to complete pending return {ReturnId}", request.ReturnId);
            return new Response.MarketplaceCompletePendingReturn
            {
                Success = false
            };
        }
    }
}
