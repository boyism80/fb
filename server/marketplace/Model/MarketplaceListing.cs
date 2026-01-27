using fb.protocol.marketplace;
using Http.Model;

namespace Marketplace.Model;

public class MarketplaceListing : BaseModel
{
    public string Id { get; set; }
    public uint World { get; set; } // World identifier (e.g., 1, 2)
    public uint SellerId { get; set; }
    public uint ItemModel { get; set; }
    public ushort RemainingCount { get; set; }
    public uint? ItemDurability { get; set; }
    public string ItemCustomName { get; set; }
    public uint Price { get; set; } // Per unit price
    public ListingState Status { get; set; }
    public DateTime ExpireDate { get; set; }
    public new DateTime CreatedDate { get; set; }
    public DateTime? SoldDate { get; set; } // Set when remaining_count becomes 0
}

