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
    public async Task<Response.List> ListItem(Request.List request)
    {
        try
        {
            if (request.ListingId == 0)
            {
                return new Response.List
                {
                    ListingId = 0,
                    Error = (uint)ErrorCode.MarketplaceListingNotFound
                };
            }

            await _marketplaceService.ListItemAsync(
                request.World,
                request.CharacterId,
                request.ListingId,
                request.Item.Model,
                request.Item.Count,
                request.Item.Durability,
                request.Item.CustomName,
                request.Item.Uid,
                request.Price,
                Fb.Model.ConstValue.Marketplace.ExpireTime,
                DateTimeOffset.FromUnixTimeMilliseconds(request.Deadline).UtcDateTime);

            return new Response.List
            {
                ListingId = request.ListingId,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning("Marketplace list validation error: {ErrorCode}", e.Error);
            return new Response.List
            {
                ListingId = 0,
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create listing for character {CharacterId}", request.CharacterId);
            return new Response.List
            {
                ListingId = 0,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("abort-list")]
    public async Task<Response.AbortList> AbortList(Request.AbortList request)
    {
        try
        {
            if (request.ListingId == 0)
                throw new LogicException(ErrorCode.MarketplaceListingNotFound);

            var created = await _marketplaceService.AbortListAsync(
                request.World,
                request.CharacterId,
                request.ListingId,
                request.Item.Model,
                request.Item.Count,
                request.Item.Durability,
                request.Item.CustomName,
                request.Item.Uid,
                request.Price);

            return new Response.AbortList
            {
                Created = created,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning("Marketplace abort list error: {ErrorCode}", e.Error);
            return new Response.AbortList
            {
                Created = false,
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to abort listing {ListingId}", request.ListingId);
            return new Response.AbortList
            {
                Created = false,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("cancel")]
    public async Task<Response.Cancel> CancelListing(Request.Cancel request)
    {
        try
        {
            await _marketplaceService.CancelListingAsync(request.World, request.CharacterId, request.ListingId);

            return new Response.Cancel
            {
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            return new Response.Cancel
            {
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to cancel listing {ListingId}", request.ListingId);
            return new Response.Cancel
            {
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("purchase")]
    public async Task<Response.Purchase> PurchaseItem(Request.Purchase request)
    {
        try
        {
            if (request.PurchaseId == 0)
                throw new LogicException(ErrorCode.MarketplaceListingNotFound);

            var result = await _marketplaceService.PurchaseItemAsync(
                request.World,
                request.BuyerId,
                request.ListingId,
                request.PurchaseCount,
                request.PurchaseId,
                DateTimeOffset.FromUnixTimeMilliseconds(request.Deadline).UtcDateTime);

            return new Response.Purchase
            {
                Item = new Protocol.Item
                {
                    Owner = request.BuyerId,
                    Model = result.Listing.ItemModel,
                    Count = result.ActualPurchaseCount,
                    Durability = result.Listing.ItemDurability,
                    CustomName = result.Listing.ItemCustomName ?? string.Empty,
                    Uid = result.Listing.ItemUid
                },
                ActualPurchaseCount = result.ActualPurchaseCount,
                RefundAmount = result.RefundAmount,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning("Marketplace purchase validation error: {ErrorCode}", e.Error);
            return new Response.Purchase
            {
                Item = new Protocol.Item
                {
                    Owner = 0,
                    Model = 0,
                    Count = 0,
                    Durability = null,
                    CustomName = string.Empty
                },
                ActualPurchaseCount = 0,
                RefundAmount = 0,
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to purchase listing {ListingId}", request.ListingId);
            return new Response.Purchase
            {
                Item = new Protocol.Item
                {
                    Owner = 0,
                    Model = 0,
                    Count = 0,
                    Durability = null,
                    CustomName = string.Empty
                },
                ActualPurchaseCount = 0,
                RefundAmount = 0,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("abort-purchase")]
    public async Task<Response.AbortPurchase> AbortPurchase(Request.AbortPurchase request)
    {
        try
        {
            if (request.PurchaseId == 0)
                throw new LogicException(ErrorCode.MarketplaceListingNotFound);

            var purchased = await _marketplaceService.AbortPurchaseAsync(
                request.World,
                request.BuyerId,
                request.ListingId,
                request.PurchaseId);

            return new Response.AbortPurchase
            {
                Purchased = purchased,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning("Marketplace abort purchase error: {ErrorCode}", e.Error);
            return new Response.AbortPurchase
            {
                Purchased = false,
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to abort purchase {PurchaseId}", request.PurchaseId);
            return new Response.AbortPurchase
            {
                Purchased = false,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("search")]
    public async Task<Response.Search> SearchItems(Request.Search request)
    {
        try
        {
            var option = new Marketplace.Model.MarketplaceSearchOption
            {
                ItemName = request.ItemName,
                MinPrice = request.MinPrice,
                MaxPrice = request.MaxPrice,
                SellerId = request.SellerId,
                SortBy = request.SortBy,
                Page = request.Page
            };

            var result = await _marketplaceService.SearchItemsAsync(option);

            var protocolListings = result.Listings.Select(l => new Protocol.Listing
            {
                Id = l.Id,
                SellerId = l.SellerId,
                Item = new Protocol.Item
                {
                    Owner = l.SellerId,
                    Model = l.ItemModel,
                    Count = l.RemainingCount,
                    Durability = l.ItemDurability,
                    CustomName = l.ItemCustomName ?? string.Empty,
                    Uid = l.ItemUid
                },
                Price = l.Price,
                ExpireDate = l.ExpireDate.ToString("yyyy-MM-dd HH:mm:ss"),
                CreatedDate = l.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")
            }).ToList();

            var searchResult = new Protocol.SearchResult
            {
                Listings = protocolListings,
                TotalCount = (uint)result.TotalCount,
                Page = option.Page
            };

            return new Response.Search
            {
                Result = searchResult,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning("Marketplace search validation error: {ErrorCode}", e.Error);
            return new Response.Search
            {
                Result = new Protocol.SearchResult
                {
                    Listings = [],
                    TotalCount = 0,
                    Page = request.Page
                },
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to search listings");
            return new Response.Search
            {
                Result = new Protocol.SearchResult
                {
                    Listings = [],
                    TotalCount = 0,
                    Page = request.Page
                },
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("get-listings")]
    public async Task<Response.GetListings> GetListings(Request.GetListings request)
    {
        try
        {
            var results = await _marketplaceService.GetListingsByIdsAsync(
                request.ListingIds ?? new List<ulong>(),
                request.BuyerId);

            var protocolListings = results.Select(r => new Protocol.Listing
            {
                Id = r.Listing.Id,
                SellerId = r.Listing.SellerId,
                Item = new Protocol.Item
                {
                    Owner = r.Listing.SellerId,
                    Model = r.Listing.ItemModel,
                    Count = r.Listing.RemainingCount,
                    Durability = r.Listing.ItemDurability,
                    CustomName = r.Listing.ItemCustomName ?? string.Empty,
                    Uid = r.Listing.ItemUid
                },
                Price = r.Listing.Price,
                State = (Protocol.ListingState)r.Listing.Status,
                ExpireDate = r.Listing.ExpireDate.ToString("yyyy-MM-dd HH:mm:ss"),
                CreatedDate = r.Listing.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss"),
                PurchaseInfo = r.Purchase != null ? new Protocol.PurchaseInfo
                {
                    PurchaseCount = r.Purchase.PurchaseCount,
                    PurchasePrice = r.Purchase.PurchasePrice,
                    CreatedDate = r.Purchase.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")
                } : null
            }).ToList();

            return new Response.GetListings
            {
                Listings = protocolListings,
                Error = (uint)ErrorCode.None
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to get listings");
            return new Response.GetListings
            {
                Listings = [],
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }
}
