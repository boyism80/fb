using Db.Reepository;
using Db.Service;
using http.Service;
using StackExchange.Redis;

namespace Db.Model
{
    public class SpellKey : IRedisCacheKey
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
        public required bool Deleted { get; set; }

        public SpellKey Key => new SpellKey
        {
            Owner = Owner,
            Slot = Slot,
        };
    }
}
