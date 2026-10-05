namespace Marketplace.Model;

public class MarketplaceDelivery
{
    public ulong Id { get; set; }
    public string ExternalRef { get; set; }
    public uint World { get; set; }
    public uint User { get; set; }
    public string Title { get; set; }
    public string Message { get; set; }
    public List<Fb.Model.Dsl> Attachments { get; set; } = new List<Fb.Model.Dsl>();
    public uint Attempts { get; set; }
    public string LastError { get; set; }
    public DateTime CreatedDate { get; set; }
    public DateTime? DeliveredDate { get; set; }
}
