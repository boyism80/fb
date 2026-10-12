using StackExchange.Redis;

namespace Http.Model
{
    public class ClanMoneyPendingKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public ulong RequestId { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:clan_money_pending:{User}";

        public RedisValue GetRedisField() => RequestId;
    }

    public class ClanMoneyPending : ClanMoneyPendingKey, IModel
    {
        public uint Clan { get; set; }
        public long ClanDelta { get; set; }
        public long MoneyDelta { get; set; }
    }
}
