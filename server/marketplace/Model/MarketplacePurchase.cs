using Http.Model;

namespace Marketplace.Model;

public class MarketplacePurchase : BaseModel
{
    public string Id { get; set; }
    public int World { get; set; } // World identifier (e.g., 1, 2)
    public string ListingId { get; set; }
    public uint BuyerId { get; set; }
    public ushort PurchaseCount { get; set; }
    public ulong PurchasePrice { get; set; }
    public new DateTime CreatedDate { get; set; }
}
