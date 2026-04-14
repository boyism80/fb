using Fb.Model;
using StackExchange.Redis;

namespace Http.Model
{
    public class StoragePendingBoxKey : BaseModel, IRedisHashKey
    {
        private const uint GlobalShardSeed = 0;
        public string Id { get; set; } = string.Empty;
        public uint? User { get; set; }

        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => User.HasValue
            ? $"fb:cache:storage:pending:{User.Value}"
            : "fb:cache:storage:pending:global";

        public RedisValue GetRedisField() => Id;
    }

    public class StoragePendingBox : StoragePendingBoxKey, IModel
    {
        public string Title { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;
        public List<Dsl> Attachments { get; set; } = new List<Dsl>();
        public DateTime? ExpiredDate { get; set; }
    }
}

