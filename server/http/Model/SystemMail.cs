using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents a system mail key model that provides Redis hash caching functionality.
    /// System mails are stored in a Redis hash with mail ID as the field.
    /// </summary>
    public class SystemMailKey : BaseModel, IRedisHashKey
    {
        /// <summary>
        /// Gets or sets the unique identifier for this system mail.
        /// </summary>
        /// <value>The unique system mail ID used for database operations.</value>
        public required uint Id { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding.
        /// System mails use a fixed hash value since they are global.
        /// </summary>
        /// <returns>A fixed hash value of 0 for global system mail storage.</returns>
        public uint GetHash() => 0;

        /// <summary>
        /// Gets the Redis key for caching system mails.
        /// </summary>
        /// <returns>A formatted Redis key string for system mail cache storage.</returns>
        public RedisKey GetRedisKey() => "cache:system_mail";

        /// <summary>
        /// Gets the Redis field identifier for this specific system mail within the hash.
        /// </summary>
        /// <returns>The system mail ID as a Redis field string.</returns>
        public RedisValue GetRedisField() => Id.ToString();
    }

    /// <summary>
    /// Represents a system mail model for broadcasting messages to all users.
    /// Contains mail content with optional expiration date for automatic cleanup.
    /// </summary>
    public class SystemMail : SystemMailKey, IModel
    {
        /// <summary>
        /// Gets or sets the title/subject of the system mail message.
        /// </summary>
        /// <value>The mail subject line displayed to all users.</value>
        public string Title { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the body content of the system mail message.
        /// </summary>
        /// <value>The full text content of the system mail message.</value>
        public string Contents { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the optional expiration date for this system mail.
        /// </summary>
        /// <value>The expiration date and time, or null if the mail never expires.</value>
        public DateTime? ExpireDate { get; set; }
    }
}

