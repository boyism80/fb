using StackExchange.Redis;

namespace Http.Model
{
    public class MarriageKey : BaseModel, IRedisValueKey
    {
        public required uint CharacterId { get; set; }

        public uint? GetHash() => CharacterId;

        public RedisKey GetRedisKey() => $"cache:marriage:{CharacterId}";
    }

    public class Marriage : MarriageKey, IModel
    {
        public uint? SpouseId { get; set; }

        public DateTime RemarriageAfter { get; set; } = DateTime.UtcNow;

        public uint DivorceCount { get; set; }
    }
}