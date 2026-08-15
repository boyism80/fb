using StackExchange.Redis;

namespace Http.Model
{
    public class ClanKey : BaseModel, IRedisValueKey
    {
        public required uint Id { get; set; }
        public uint? GetHash() => Id;

        public RedisKey GetRedisKey() => $"fb:cache:clan:{Id}";
    }

    public class Clan : ClanKey, IModel
    {
        public string Name { get; set; }
        public string Title { get; set; }
        public ulong Money { get; set; }
        public static string DistributedLockKey(uint id)
        {
            return $"fb:lock:clan:{id}";
        }
    }
}
