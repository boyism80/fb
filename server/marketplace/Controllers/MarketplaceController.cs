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

    [HttpPost("allocate-listing-id")]
    public async Task<Response.AllocateListingId> AllocateListingId(Request.AllocateListingId request)
    {
        try
        {
            var listingId = await _marketplaceService.AllocateListingIdAsync(request.CharacterId);

            return new Response.AllocateListingId
            {
                ListingId = listingId,
                Error = (uint)ErrorCode.None
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to allocate listing ID for character {CharacterId}", request.CharacterId);
            return new Response.AllocateListingId
            {
                ListingId = string.Empty,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("list")]
    public async Task<Response.List> ListItem(Request.List request)
    {
        try
        {
            var listing = await _marketplaceService.ListItemAsync(
                request.CharacterId,
                request.ListingId ?? string.Empty,
                request.Item.Model,
                request.Item.Count,
                request.Item.Durability,
                request.Item.CustomName ?? string.Empty,
                request.Price,
                request.ExpireHours) ?? throw new LogicException(ErrorCode.Unhandled);

            // Return listing_id (UUID)
            return new Response.List
            {
                ListingId = listing.Id ?? string.Empty,
                ListingFee = listing.ListingFee,
                Error = (uint)ErrorCode.None
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create listing for character {CharacterId}", request.CharacterId);
            return new Response.List
            {
                ListingId = string.Empty,
                ListingFee = 0,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("cancel")]
    public async Task<Response.Cancel> CancelListing(Request.Cancel request)
    {
        try
        {
            await _marketplaceService.CancelListingAsync(request.CharacterId, request.ListingId);

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
            var listing = await _marketplaceService.PurchaseItemAsync(request.BuyerId, request.ListingId);

            if (listing != null)
            {
                return new Response.Purchase
                {
                    Item = new Protocol.Item
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
                Error = errorCode
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
                    Listings = new List<Protocol.Listing>(),
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
                    Listings = new List<Protocol.Listing>(),
                    TotalCount = 0,
                    Page = request.Page
                },
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("check-status")]
    public async Task<Response.CheckListingStatus> CheckListingStatus(Request.CheckListingStatus request)
    {
        try
        {
            var listing = await _marketplaceService.CheckListingStatusAsync(request.ListingId);

            if (listing != null)
            {
                return new Response.CheckListingStatus
                {
                    ListingId = listing.Id ?? string.Empty,
                    ListingFee = listing.ListingFee,
                    Error = (uint)ErrorCode.None
                };
            }

            return new Response.CheckListingStatus
            {
                ListingId = string.Empty,
                ListingFee = 0,
                Error = (uint)ErrorCode.None
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to check listing status for listing {ListingId}", request.ListingId);
            return new Response.CheckListingStatus
            {
                ListingId = string.Empty,
                ListingFee = 0,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

}
