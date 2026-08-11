using StackExchange.Redis;

namespace Http.Model
{
    public class CastleKey : BaseModel, IRedisValueKey
    {
        public required byte DivineBeast { get; set; }
        public uint? GetHash() => DivineBeast;

        public RedisKey GetRedisKey() => $"fb:cache:castle:{DivineBeast}";
    }

    public class Castle : CastleKey, IModel
    {
        public uint? OwnerClan { get; set; }
        public static string DistributedLockKey(byte divineBeast)
        {
            return $"fb:lock:castle:{divineBeast}";
        }
    }
}
