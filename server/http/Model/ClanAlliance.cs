using StackExchange.Redis;

namespace Http.Model
{
    public class ClanAllianceKey : BaseModel, IRedisValueKey
    {
        public required uint Clan { get; set; }
        public uint? GetHash() => Clan;

        public RedisKey GetRedisKey() => $"fb:cache:clan-alliance:{Clan}";
    }

    public class ClanAlliance : ClanAllianceKey, IModel
    {
        public uint AlliedClan { get; set; }
        public static string DistributedLockKey(uint clan)
        {
            return $"fb:lock:clan-alliance:{clan}";
        }
    }
}
