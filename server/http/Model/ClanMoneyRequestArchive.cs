using StackExchange.Redis;

namespace Http.Model
{
    public class ClanMoneyRequestArchiveKey : BaseModel, IRedisHashKey
    {
        public required uint Clan { get; set; }
        public ulong RequestId { get; set; }
        public uint? GetHash() => Clan;

        public RedisKey GetRedisKey() => $"fb:cache:clan_money_request_archive:{Clan}";

        public RedisValue GetRedisField() => RequestId;
    }

    public class ClanMoneyRequestArchive : ClanMoneyRequestArchiveKey, IModel
    {
        public bool Applied { get; set; }
    }
}
