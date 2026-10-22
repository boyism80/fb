using StackExchange.Redis;

namespace Http.Model
{
    public enum UnsettledType : byte
    {
        ClanMoney = 0,
        StorageClaim = 1,
        MarketList = 2,
        MarketPurchase = 3,
        ClanFee = 4,
    }

    public class UnsettledRequestKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public ulong Id { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:unsettled_request:{User}";

        public RedisValue GetRedisField() => Id;
    }

    public class UnsettledRequest : UnsettledRequestKey, IModel
    {
        public byte Type { get; set; }
        public ulong Target { get; set; }
        public string Detail { get; set; } = "{}";
    }
}
