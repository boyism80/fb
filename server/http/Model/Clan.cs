using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents a clan key model that provides Redis value caching functionality.
    /// Serves as a base class for clan-related models with Redis integration.
    /// </summary>
    public class ClanKey : BaseModel, IRedisValueKey
    {
        /// <summary>
        /// Gets or sets the unique identifier of the clan.
        /// </summary>
        /// <value>The clan's unique ID used for database and cache operations.</value>
        public required uint Id { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the clan ID.
        /// </summary>
        /// <returns>The clan ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Id;

        /// <summary>
        /// Gets the Redis key for caching this clan's data.
        /// </summary>
        /// <returns>A formatted Redis key string for clan cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:clan:{Id}";
    }

    /// <summary>
    /// Represents a complete clan model with identification and distributed locking support.
    /// Contains clan name, title information and provides thread-safe operations.
    /// </summary>
    public class Clan : ClanKey, IModel
    {
        /// <summary>
        /// Gets or sets the name of the clan.
        /// </summary>
        /// <value>The clan's display name used throughout the game.</value>
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the title or motto of the clan.
        /// </summary>
        /// <value>The clan's title or descriptive motto text.</value>
        public string Title { get; set; }

        /// <summary>
        /// Generates a distributed lock key for clan operations.
        /// Used to ensure thread-safe clan modifications across distributed systems.
        /// </summary>
        /// <param name="id">The clan ID to generate a lock key for.</param>
        /// <returns>A formatted distributed lock key string for the specified clan.</returns>
        public static string DistributedLockKey(uint id)
        {
            return $"lock:clan:{id}";
        }
    }
}
