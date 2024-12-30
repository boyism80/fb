using StackExchange.Redis;

namespace Http.Model
{
    public class TraceKey : BaseModel, IRedisHashKey
    {
        public required uint Uid { get; set; }
        public uint Model { get; set; }

        public uint GetHash() => Uid;
        public RedisKey GetRedisKey() => $"cache:trace:{Uid}";
        public RedisValue GetRedisField() => $"{Model}";
    }

    public class Trace : TraceKey, IModel
    {
        public string Text { get; set; }
    }
}
