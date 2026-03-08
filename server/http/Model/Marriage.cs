using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Key for marriage entity. Sharding is by character id (same shard as the character).
    /// </summary>
    public class MarriageKey : BaseModel, IRedisValueKey
    {
        /// <summary>
        /// Character id (primary key of marriage row).
        /// </summary>
        public required uint CharacterId { get; set; }

        /// <summary>
        /// Gets the hash value for sharding (character id).
        /// </summary>
        public uint? GetHash() => CharacterId;

        /// <summary>
        /// Gets the Redis key for caching marriage state.
        /// </summary>
        public RedisKey GetRedisKey() => $"cache:marriage:{CharacterId}";
    }

    /// <summary>
    /// Marriage state for a character: spouse id, remarriage cooldown, and divorce count.
    /// Maps to the marriage table (one row per character in the marriage system).
    /// </summary>
    public class Marriage : MarriageKey, IModel
    {
        /// <summary>
        /// Character id of the spouse. Null when single.
        /// </summary>
        public uint? SpouseId { get; set; }

        /// <summary>
        /// After this time the character can marry again (set on divorce).
        /// When there is no cooldown, set to current time so that RemarriageAfter &lt;= UtcNow allows remarriage.
        /// </summary>
        public DateTime RemarriageAfter { get; set; } = DateTime.UtcNow;

        /// <summary>
        /// Number of divorces (for title/achievement).
        /// </summary>
        public uint DivorceCount { get; set; }
    }
}
