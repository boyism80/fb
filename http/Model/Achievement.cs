using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents an achievement key model that provides Redis hash caching functionality.
    /// Serves as a base class for achievement-related models with Redis integration.
    /// </summary>
    public class AchievementKey : BaseModel, IRedisHashKey
    {
        /// <summary>
        /// Gets or sets the unique identifier of the character who earned this achievement.
        /// </summary>
        /// <value>The character's unique ID associated with this achievement.</value>
        public required uint Uid { get; set; }

        /// <summary>
        /// Gets or sets the achievement model identifier.
        /// </summary>
        /// <value>The unique model ID that defines the achievement type and requirements.</value>
        public uint Model { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the character ID.
        /// </summary>
        /// <returns>The character ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Uid;

        /// <summary>
        /// Gets the Redis key for caching this character's achievements.
        /// </summary>
        /// <returns>A formatted Redis key string for achievement cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:achievement:{Uid}";

        /// <summary>
        /// Gets the Redis field identifier for this specific achievement within the hash.
        /// </summary>
        /// <returns>A formatted Redis field string based on the achievement model ID.</returns>
        public RedisValue GetRedisField() => $"{Model}";
    }

    /// <summary>
    /// Represents a complete achievement model with descriptive text.
    /// Contains achievement information and any associated custom text or notes.
    /// </summary>
    public class Achievement : AchievementKey, IModel
    {
        /// <summary>
        /// Gets or sets the descriptive text or notes associated with this achievement.
        /// </summary>
        /// <value>Custom text describing the achievement or player notes.</value>
        public string Text { get; set; }

        /// <summary>
        /// Gets or sets the icon of the achievement.
        /// </summary>
        /// <value>The icon of the achievement.</value>
        public byte? Icon { get; set; }

        /// <summary>
        /// Gets or sets the color of the achievement.
        /// </summary>
        /// <value>The color of the achievement.</value>
        public ushort? Color { get; set; }
    }
}
