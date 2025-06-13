using StackExchange.Redis;

namespace Http.Model
{
    public class AchievementKey : BaseModel, IRedisHashKey
    {
        public required uint Uid { get; set; }
        public uint Model { get; set; }

        public uint GetHash() => Uid;
        public RedisKey GetRedisKey() => $"cache:achievement:{Uid}";
        public RedisValue GetRedisField() => $"{Model}";
    }

    public class Achievement : AchievementKey, IModel
    {
        public string Text { get; set; }
    }
}
