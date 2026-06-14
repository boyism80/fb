using Fb.Model;
using StackExchange.Redis;

namespace Http.Model
{
    public class MarketplacePendingKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public string PendingKey { get; set; } = string.Empty;
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:marketplace_pending:{User}";

        public RedisValue GetRedisField() => PendingKey;
    }

    public class MarketplacePending : MarketplacePendingKey, IModel
    {
        public byte Type { get; set; }
        public string PurchaseId { get; set; } = string.Empty;
        public string ListingId { get; set; } = string.Empty;
        public List<Dsl> Attachments { get; set; } = new List<Dsl>();
        public ushort ExpectedPurchaseCount { get; set; }
        public uint ExpectedTotalPrice { get; set; }
        public uint CharacterId { get; set; }
    }
}
