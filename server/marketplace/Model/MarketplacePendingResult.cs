namespace Marketplace.Model;

public class MarketplacePendingResult
{
    public List<PendingTransaction> Transactions { get; set; }
    public List<PendingReturn> Returns { get; set; }
}

