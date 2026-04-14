using StackExchange.Redis;

namespace Http.Model
{
    public class CharacterSyncKey : BaseModel, IRedisValueKey
    {
        public required uint Uid { get; set; }
        public uint? GetHash() => Uid;

        public RedisKey GetRedisKey() => $"fb:cache:character-sync:{Uid}";
    }

    public class CharacterSync : CharacterSyncKey, IModel
    {
        public uint? Group { get; set; }
        public uint? Clan { get; set; }
        public static string DistributedLockKey(uint uid)
        {
            return $"fb:lock:character-sync:{uid}";
        }
    }
}
