using StackExchange.Redis;

namespace Http.Model
{
    public class FriendKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public uint FriendUid { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:friend:{User}";

        public RedisValue GetRedisField() => $"{FriendUid}";
    }

    public class Friend : FriendKey, IModel
    {
        public string FriendName { get; set; } = string.Empty;
        public bool Mutual { get; set; }

        public static string DistributedLockKey(uint user)
        {
            return $"fb:lock:friend:{user}";
        }
    }
}
