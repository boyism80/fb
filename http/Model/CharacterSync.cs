using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents a character synchronization key model that provides Redis value caching functionality.
    /// Serves as a base class for character sync-related models with Redis integration.
    /// </summary>
    public class CharacterSyncKey : BaseModel, IRedisValueKey
    {
        /// <summary>
        /// Gets or sets the unique identifier of the character for synchronization.
        /// </summary>
        /// <value>The character's unique ID used for sync operations.</value>
        public required uint Uid { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the character ID.
        /// </summary>
        /// <returns>The character ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Uid;

        /// <summary>
        /// Gets the Redis key for caching this character's synchronization data.
        /// </summary>
        /// <returns>A formatted Redis key string for character sync cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:character-sync:{Uid}";
    }

    /// <summary>
    /// Represents a complete character synchronization model with group and clan associations.
    /// Contains character's current group and clan memberships for cross-server synchronization.
    /// </summary>
    public class CharacterSync : CharacterSyncKey, IModel
    {
        /// <summary>
        /// Gets or sets the unique identifier of the group this character belongs to.
        /// </summary>
        /// <value>The group ID if the character is in a group; otherwise, null.</value>
        public uint? Group { get; set; }

        /// <summary>
        /// Gets or sets the unique identifier of the clan this character belongs to.
        /// </summary>
        /// <value>The clan ID if the character is in a clan; otherwise, null.</value>
        public uint? Clan { get; set; }

        /// <summary>
        /// Generates a distributed lock key for character synchronization operations.
        /// Used to ensure thread-safe character sync modifications across distributed systems.
        /// </summary>
        /// <param name="uid">The character ID to generate a lock key for.</param>
        /// <returns>A formatted distributed lock key string for the specified character.</returns>
        public static string DistributedLockKey(uint uid)
        {
            return $"lock:character-sync:{uid}";
        }
    }
}
