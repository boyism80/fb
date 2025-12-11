namespace Marketplace.Model;

public class PendingReturn
{
    public ulong Id { get; set; }
    public ulong ListingId { get; set; }
    public uint SellerId { get; set; }
    public uint ItemModel { get; set; }
    public ushort ItemCount { get; set; }
    public uint? ItemDurability { get; set; }
    public string ItemCustomName { get; set; }
    public DateTime CreatedDate { get; set; }
}

