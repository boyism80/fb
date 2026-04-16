using StackExchange.Redis;

namespace Http.Model
{
    public class CharacterRealtimeStateKey : BaseModel, IRedisValueKey
    {
        public required uint Uid { get; set; }
        public uint? GetHash() => Uid;

        public RedisKey GetRedisKey() => $"fb:cache:character-realtime-state:{Uid}";
    }

    public class CharacterRealtimeState : CharacterRealtimeStateKey, IModel
    {
        public uint? Group { get; set; }
        public uint? Clan { get; set; }
        public static string DistributedLockKey(uint uid)
        {
            return $"fb:lock:character-realtime-state:{uid}";
        }
    }
}

