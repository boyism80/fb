using Http.Model;

namespace Marketplace.Model;

public class MarketplacePurchase : BaseModel
{
    public string Id { get; set; }
    public string ListingId { get; set; }
    public uint BuyerId { get; set; }
    public ushort PurchaseCount { get; set; }
    public uint PurchasePrice { get; set; }
    public new DateTime CreatedDate { get; set; }
}
