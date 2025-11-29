using StackExchange.Redis;

namespace Http.Model
{
    public class BanKey : BaseModel, IRedisValueKey
    {
        public required uint User { get; set; }
        public uint GetHash() => 0;

        public RedisKey GetRedisKey() => $"cache:ban:{User}";
    }

    public class Ban : BanKey, IModel
    {
        public string Reason { get; set; }
        public DateTime? ExpireDate { get; set; }
    }
}

