namespace Marketplace.Model;

public class MarketplaceSearchOption
{
    public string ItemName { get; set; }
    public ulong? MinPrice { get; set; }
    public ulong? MaxPrice { get; set; }
    public uint? SellerId { get; set; }
    public string SortBy { get; set; }
    public uint Page { get; set; } = 1;
}

