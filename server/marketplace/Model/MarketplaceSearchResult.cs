namespace Marketplace.Model;

public class MarketplaceSearchResult
{
    public List<MarketplaceListing> Listings { get; set; } = new();
    public int TotalCount { get; set; }
}

