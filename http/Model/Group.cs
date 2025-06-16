using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents a group key model that provides Redis value caching functionality.
    /// Serves as a base class for group-related models with Redis integration.
    /// </summary>
    public class GroupKey : BaseModel, IRedisValueKey
    {
        /// <summary>
        /// Gets or sets the unique identifier of the group master (leader).
        /// </summary>
        /// <value>The group master's unique character ID.</value>
        public required uint Master { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the master ID.
        /// </summary>
        /// <returns>The master ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Master;

        /// <summary>
        /// Gets the Redis key for caching this group's data.
        /// </summary>
        /// <returns>A formatted Redis key string for group cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:group:{Master}";
    }

    /// <summary>
    /// Represents a complete group model with member management functionality.
    /// Contains the list of group members and provides distributed locking support.
    /// </summary>
    public class Group : GroupKey, IModel
    {
        /// <summary>
        /// Gets or sets the list of group member character IDs.
        /// </summary>
        /// <value>A collection of unique character IDs representing group members.</value>
        public List<uint> Members { get; set; } = new List<uint>();

        /// <summary>
        /// Generates a distributed lock key for group operations.
        /// Used to ensure thread-safe group modifications across distributed systems.
        /// </summary>
        /// <param name="id">The group master ID to generate a lock key for.</param>
        /// <returns>A formatted distributed lock key string for the specified group.</returns>
        public static string DistributedLockKey(uint id)
        {
            return $"lock:group:{id}";
        }
    }
}
