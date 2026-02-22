using StackExchange.Redis;

namespace Http.Model
{
    public class AchievementKey : BaseModel, IRedisHashKey
    {
        public required uint Uid { get; set; }
        public uint Id { get; set; }
        public uint? GetHash() => Uid;

        public RedisKey GetRedisKey() => $"cache:achievement:{Uid}";

        public RedisValue GetRedisField() => $"{Id}";
    }

    public class Achievement : AchievementKey, IModel
    {
        public string Text { get; set; } = string.Empty;
        public byte Icon { get; set; }
        public ushort Color { get; set; }
    }
}
