using StackExchange.Redis;

namespace Http.Model
{
    public class EscrowKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public ulong Id { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:escrow:{User}";

        public RedisValue GetRedisField() => Id;
    }

    public class Escrow : EscrowKey, IModel
    {
        public ulong Money { get; set; }
    }
}
