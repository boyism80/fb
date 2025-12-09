using StackExchange.Redis;

namespace Http.Model
{
    public class StorageRewardMarkKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public string PendingId { get; set; } = string.Empty;
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"cache:storage:mark:{User}";

        public RedisValue GetRedisField() => PendingId;
    }

    public class StorageRewardMark : StorageRewardMarkKey, IModel
    {
        public DateTime? ExpiredDate { get; set; }
    }
}

