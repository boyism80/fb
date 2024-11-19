using StackExchange.Redis;

namespace Db.Model
{
    public class SpellKey : BaseModel, IRedisHashKey
    {
        public required uint Owner { get; set; }
        public byte Slot { get; set; }

        public uint GetDbKey() => Owner;
        public RedisKey GetRedisKey() => $"cache:spell:{Owner}";
        public RedisValue GetRedisField() => $"{Slot}";
    }

    public class Spell : SpellKey, IModel
    {
        public required int Model { get; set; }

        public SpellKey Key => new SpellKey
        {
            Owner = Owner,
            Slot = Slot,
        };
    }
}
