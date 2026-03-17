using StackExchange.Redis;

namespace Http.Model
{
    public class SpellKey : BaseModel, IRedisHashKey
    {
        public required uint Owner { get; set; }
        public byte Slot { get; set; }
        public uint? GetHash() => Owner;

        public RedisKey GetRedisKey() => $"cache:spell:{Owner}";

        public RedisValue GetRedisField() => $"{Slot}";
    }

    public class Spell : SpellKey, IModel
    {
        public required int Model { get; set; }
        public DateTime Next { get; set; }
    }
}