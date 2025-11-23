using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents a ban key model that provides Redis caching functionality.
    /// Serves as a base class for ban-related models with Redis integration.
    /// Ban table is stored in global DB (no sharding).
    /// </summary>
    public class BanKey : BaseModel, IRedisValueKey
    {
        /// <summary>
        /// Gets or sets the unique identifier for the banned user.
        /// </summary>
        /// <value>The user's unique ID used for database and cache operations.</value>
        public required uint User { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding.
        /// Always returns 0 since ban table is in global DB (no sharding).
        /// </summary>
        /// <returns>0 for global DB access.</returns>
        public uint GetHash() => 0;

        /// <summary>
        /// Gets the Redis key for caching this ban.
        /// </summary>
        /// <returns>A formatted Redis key string for ban cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:ban:{User}";
    }

    /// <summary>
    /// Represents a complete ban model with all ban-related properties.
    /// Contains ban reason, expiration date, and user information.
    /// </summary>
    public class Ban : BanKey, IModel
    {
        /// <summary>
        /// Gets or sets the reason for the ban.
        /// </summary>
        /// <value>The ban reason description.</value>
        public string Reason { get; set; }

        /// <summary>
        /// Gets or sets the expiration date of the ban.
        /// </summary>
        /// <value>The expiration date, or null for permanent ban.</value>
        public DateTime? ExpireDate { get; set; }
    }
}

