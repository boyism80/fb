using StackExchange.Redis;

namespace Http.Model
{
    public class QuestKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public uint Id { get; set; }
        public uint? GetHash() => User;
        public RedisKey GetRedisKey() => $"fb:cache:quest:{User}";
        public RedisValue GetRedisField() => Id.ToString();
    }

    public class Quest : QuestKey, IModel
    {
        public required uint Step { get; set; }
        public required uint Progress { get; set; }
        public required string Param { get; set; }
        public required bool Completed { get; set; }
    }
}