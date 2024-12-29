using StackExchange.Redis;

namespace Http.Model
{
    public class GroupKey : BaseModel, IRedisValueKey
    {
        public required uint Master { get; set; }

        public uint GetDbKey() => Master;
        public RedisKey GetRedisKey() => $"cache:group:{Master}";
    }

    public class Group : GroupKey, IModel
    {
        public List<uint> Members { get; set; } = new List<uint>();

        public static string DistributedLockKey(uint id)
        {
            return $"lock:group:{id}";
        }
    }
}
