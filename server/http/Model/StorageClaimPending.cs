using StackExchange.Redis;

namespace Http.Model
{
    public class StorageClaimPendingKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public uint Box { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:storage_claim_pending:{User}";

        public RedisValue GetRedisField() => Box;
    }

    public class StorageClaimPending : StorageClaimPendingKey, IModel
    {
        public ulong ClaimId { get; set; }
    }
}
