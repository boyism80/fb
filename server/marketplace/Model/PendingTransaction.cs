namespace Marketplace.Model;

public class PendingTransaction
{
    public ulong Id { get; set; }
    public ulong ListingId { get; set; }
    public uint BuyerId { get; set; }
    public uint SellerId { get; set; }
    public uint ItemModel { get; set; }
    public ushort ItemCount { get; set; }
    public uint Price { get; set; }
    public uint SellerRevenue { get; set; }
    public DateTime CreatedDate { get; set; }
}

