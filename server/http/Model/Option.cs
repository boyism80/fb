using StackExchange.Redis;

namespace Http.Model
{
    public class OptionKey : BaseModel, IRedisValueKey
    {
        public required uint Uid { get; set; }
        public uint? GetHash() => Uid;

        public RedisKey GetRedisKey() => $"cache:option:{Uid}";
    }

    public class Option : OptionKey, IModel
    {
        public bool Whisper { get; set; } = true;
        public bool Group { get; set; } = true;
        public bool Roar { get; set; } = true;
        public bool RoarWorlds { get; set; } = true;
        public bool MagicEffect { get; set; } = true;
        public bool WeatherEffect { get; set; } = true;
        public bool FixedMove { get; set; } = true;
        public bool Trade { get; set; } = true;
        public bool FastMove { get; set; } = true;
        public bool EffectSound { get; set; } = true;
        public bool PkProtect { get; set; } = true;
    }
}