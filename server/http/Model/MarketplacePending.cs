using Fb.Model;
using StackExchange.Redis;

namespace Http.Model
{
    public class MarketplacePendingKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public ulong PendingKey { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:marketplace_pending:{User}";

        public RedisValue GetRedisField() => PendingKey;
    }

    public class MarketplacePending : MarketplacePendingKey, IModel
    {
        public byte Type { get; set; }
        public ulong PurchaseId { get; set; }
        public ulong ListingId { get; set; }
        public List<Dsl> Attachments { get; set; } = new List<Dsl>();
        public ushort ExpectedPurchaseCount { get; set; }
        public ulong ExpectedTotalPrice { get; set; }
        public uint CharacterId { get; set; }
    }
}
