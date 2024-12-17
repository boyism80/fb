using StackExchange.Redis;

namespace Http.Model
{
    public class OptionKey : BaseModel, IRedisValueKey
    {
        public required uint Uid { get; set; }

        public uint GetDbKey() => Uid;
        public RedisKey GetRedisKey() => $"cache:option:{Uid}";
    }

    public class Option : OptionKey, IModel
    {
        public bool Whisper { get; set; }
        public bool Group { get; set; }
        public bool Roar { get; set; }
        public bool RoarWorlds { get; set; }
        public bool MagicEffect { get; set; }
        public bool WeatherEffect { get; set; }
        public bool FixedMove { get; set; }
        public bool Trade { get; set; }
        public bool FastMove { get; set; }
        public bool EffectSound { get; set; }
        public bool PkProtect { get; set; }
    }
}