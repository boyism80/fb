using StackExchange.Redis;
using System.ComponentModel.DataAnnotations.Schema;

namespace Http.Model
{
    public class CollectionUnlockKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }

        [Column("mob_id")]
        public uint MobId { get; set; }
        public uint? GetHash() => User;
        public RedisKey GetRedisKey() => $"fb:cache:collection_unlock:{User}";
        public RedisValue GetRedisField() => MobId.ToString();
    }

    public class CollectionUnlock : CollectionUnlockKey, IModel
    {
    }
}
