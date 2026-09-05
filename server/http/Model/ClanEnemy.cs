using StackExchange.Redis;

namespace Http.Model
{
    public class ClanEnemyKey : BaseModel, IRedisHashKey
    {
        public required uint Clan { get; set; }
        public uint EnemyClan { get; set; }
        public uint? GetHash() => Clan;

        public RedisKey GetRedisKey() => $"fb:cache:clan-enemy:{Clan}";

        public RedisValue GetRedisField() => $"{EnemyClan}";
    }

    public class ClanEnemy : ClanEnemyKey, IModel
    {
    }
}
