using StackExchange.Redis;

namespace Http.Model
{
    public class GroupKey : BaseModel, IRedisValueKey
    {
        public required ulong Id { get; set; }

        public uint? GetHash() => (uint)(Id % uint.MaxValue);

        public RedisKey GetRedisKey() => $"fb:cache:group:{Id}";
    }

    public class Group : GroupKey, IModel
    {
        public uint Master { get; set; }
        public List<uint> Members { get; set; } = new List<uint>();
        public ulong Revision { get; set; }

        public static string DistributedLockKey(ulong id)
        {
            return $"fb:lock:group:{id}";
        }
    }
}
