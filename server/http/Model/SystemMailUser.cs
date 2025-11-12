using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents a system mail user key model that provides Redis hash caching functionality.
    /// System mail user data is stored in a Redis hash with mail ID as the field.
    /// </summary>
    public class SystemMailUserKey : BaseModel, IRedisHashKey
    {
        /// <summary>
        /// Gets or sets the unique identifier for the user.
        /// </summary>
        /// <value>The unique user ID used for database operations.</value>
        public required uint User { get; set; }

        /// <summary>
        /// Gets or sets the system mail ID that the user has received.
        /// </summary>
        /// <value>The unique system mail ID.</value>
        public required uint MailId { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding.
        /// Uses the user ID for sharding to distribute load across Redis instances.
        /// </summary>
        /// <returns>The user ID as the hash value for Redis sharding.</returns>
        public uint GetHash() => User;

        /// <summary>
        /// Gets the Redis key for caching system mail user data.
        /// </summary>
        /// <returns>A formatted Redis key string for system mail user cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:system_mail_user:{User}";

        /// <summary>
        /// Gets the Redis field identifier for this specific system mail user within the hash.
        /// </summary>
        /// <returns>The system mail ID as a Redis field string.</returns>
        public RedisValue GetRedisField() => MailId.ToString();
    }

    /// <summary>
    /// Represents a system mail user model for tracking which system mails a user has received.
    /// Contains read status, expiration date, and deletion flag for each system mail.
    /// </summary>
    public class SystemMailUser : SystemMailUserKey, IModel
    {
        /// <summary>
        /// Gets or sets a value indicating whether this system mail has been read by the user.
        /// </summary>
        /// <value>True if the system mail has been read; otherwise, false.</value>
        public bool Read { get; set; }

        /// <summary>
        /// Gets or sets the optional expiration date for this system mail.
        /// Copied from the system mail to track expiration in user's memory.
        /// </summary>
        /// <value>The expiration date and time, or null if the mail never expires.</value>
        public DateTime? ExpireDate { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether this system mail is marked as deleted.
        /// </summary>
        /// <value>True if the system mail is soft-deleted; otherwise, false.</value>
        public bool Deleted { get; set; }

        /// <summary>
        /// Gets or sets the date and time when the system mail was created for this user.
        /// </summary>
        /// <value>The creation timestamp of the system mail user record.</value>
        public DateTime CreatedDate { get; set; }
    }
}

