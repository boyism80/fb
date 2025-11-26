using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents the Redis hash key for tracking pending reward processing per user.
    /// Maps a pending reward identifier to the user that processed it.
    /// </summary>
    public class StorageRewardMarkKey : BaseModel, IRedisHashKey
    {
        /// <summary>
        /// Gets or sets the user identifier that processed the pending reward.
        /// </summary>
        /// <value>The target user id that owns the mark row.</value>
        public required uint User { get; set; }

        /// <summary>
        /// Gets or sets the pending reward identifier.
        /// </summary>
        /// <value>The identifier of the pending record that has been applied.</value>
        public ulong PendingId { get; set; }

        /// <inheritdoc/>
        public uint GetHash() => User;

        /// <inheritdoc/>
        public RedisKey GetRedisKey() => $"cache:storage:mark:{User}";

        /// <inheritdoc/>
        public RedisValue GetRedisField() => PendingId.ToString();
    }

    /// <summary>
    /// Represents the processed state of a pending reward for a specific user.
    /// Includes expiration to support garbage collection of outdated marks.
    /// </summary>
    public class StorageRewardMark : StorageRewardMarkKey, IModel
    {
        /// <summary>
        /// Gets or sets the expiration timestamp that matches the pending entry.
        /// </summary>
        /// <value>The date when the mark becomes obsolete.</value>
        public DateTime? ExpiredDate { get; set; }
    }
}

