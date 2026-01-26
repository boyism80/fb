using Http.Model;

namespace Marketplace.Model;

public class MarketplacePurchase : BaseModel
{
    public string Id { get; set; }
    public string Section { get; set; } = string.Empty; // Section identifier (e.g., "section-1", "section-2")
    public string ListingId { get; set; }
    public uint BuyerId { get; set; }
    public ushort PurchaseCount { get; set; }
    public uint PurchasePrice { get; set; }
    public new DateTime CreatedDate { get; set; }
}
