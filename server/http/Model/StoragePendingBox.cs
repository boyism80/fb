using System.Collections.Generic;
using Fb.Model;
using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents the Redis hash key for pending storage rewards published by the operation tool.
    /// Supports both global entries (user is null) and per-user pending rewards.
    /// </summary>
    public class StoragePendingBoxKey : BaseModel, IRedisHashKey
    {
        private const uint GlobalShardSeed = 0;

        /// <summary>
        /// Gets or sets the unique identifier of the pending reward entry.
        /// </summary>
        /// <value>The auto-incremented id generated globally.</value>
        public ulong Id { get; set; }

        /// <summary>
        /// Gets or sets the user identifier that should receive the reward.
        /// </summary>
        /// <value>The target user id, or null for global broadcasts.</value>
        public uint? User { get; set; }

        /// <inheritdoc/>
        public uint GetHash() => User ?? GlobalShardSeed;

        /// <inheritdoc/>
        public RedisKey GetRedisKey() => User.HasValue
            ? $"cache:storage:pending:{User.Value}"
            : "cache:storage:pending:global";

        /// <inheritdoc/>
        public RedisValue GetRedisField() => Id.ToString();
    }

    /// <summary>
    /// Represents pending reward information before it is merged into a user's storage box.
    /// Contains the reward payload, expiration, and logical deletion flags.
    /// </summary>
    public class StoragePendingBox : StoragePendingBoxKey, IModel
    {
        /// <summary>
        /// Gets or sets the title that will be displayed in the user's storage box.
        /// </summary>
        /// <value>A concise summary of the reward.</value>
        public string Title { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the message that will be displayed in the user's storage box.
        /// </summary>
        /// <value>A localized text describing the pending reward.</value>
        public string Message { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the reward payload represented as DSL descriptors.
        /// </summary>
        /// <value>A collection of DSL items serialized as JSON for persistence.</value>
        public List<Dsl> Attachments { get; set; } = new List<Dsl>();

        /// <summary>
        /// Gets or sets the earliest time when the pending reward becomes available.
        /// </summary>
        /// <value>The gateway process will ignore the reward before this timestamp.</value>
        public DateTime? ExpiredDate { get; set; }
    }
}

