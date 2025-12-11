using Http.Model;

namespace Marketplace.Model;

public class MarketplaceListing : BaseModel
{
    public ulong Id { get; set; }
    public uint SellerId { get; set; }
    public string RequestId { get; set; }
    public uint ItemModel { get; set; }
    public ushort ItemCount { get; set; }
    public uint? ItemDurability { get; set; }
    public string ItemCustomName { get; set; }
    public uint Price { get; set; }
    public uint ListingFee { get; set; }
    public uint TransactionFee { get; set; }
    public byte Status { get; set; } // 0=Active, 1=Sold, 2=Cancelled, 3=Expired
    public DateTime ExpireDate { get; set; }
    public new DateTime CreatedDate { get; set; }
    public DateTime? SoldDate { get; set; }
    public uint? BuyerId { get; set; }
}

