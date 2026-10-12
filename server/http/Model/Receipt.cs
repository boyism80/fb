using StackExchange.Redis;

namespace Http.Model
{
    public enum ReceiptType : byte
    {
        ClanMoney = 0,
        StorageClaim = 1,
        MarketList = 2,
        MarketPurchase = 3,
    }

    public class ReceiptKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public ulong Id { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:receipt:{User}";

        public RedisValue GetRedisField() => Id;
    }

    public class Receipt : ReceiptKey, IModel
    {
        public byte Type { get; set; }
        public ulong Target { get; set; }
        public string Detail { get; set; } = "{}";
    }
}
