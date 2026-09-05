using StackExchange.Redis;

namespace Http.Model
{
    public class CastleKey : BaseModel, IRedisValueKey
    {
        public required byte DivineBeast { get; set; }
        // World-global: castle ownership is shared across all game hosts in a world.
        // Clan FK cannot span DBs (clan lives in sharded world-data), so app-layer
        // validation in CastleController is the source of truth.
        public uint? GetHash() => null;

        public RedisKey GetRedisKey() => $"fb:cache:castle:{DivineBeast}";
    }

    public class Castle : CastleKey, IModel
    {
        public uint? OwnerClan { get; set; }
        public static string DistributedLockKey(byte divineBeast)
        {
            return $"fb:lock:castle:{divineBeast}";
        }
    }
}
