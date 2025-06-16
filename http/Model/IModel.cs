using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Defines the base contract for all model entities in the system.
    /// Provides common properties for tracking entity lifecycle and state.
    /// </summary>
    public interface IModel
    {
        /// <summary>
        /// Gets or sets the date and time when the entity was created.
        /// </summary>
        /// <value>The creation timestamp of the entity.</value>
        DateTime CreatedDate { get; set; }

        /// <summary>
        /// Gets or sets the date and time when the entity was last updated.
        /// </summary>
        /// <value>The last modification timestamp of the entity.</value>
        DateTime UpdatedDate { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the entity is marked as deleted.
        /// </summary>
        /// <value>True if the entity is soft-deleted; otherwise, false.</value>
        bool Deleted { get; set; }
    }

    /// <summary>
    /// Defines the contract for model keys that provide hash-based sharding functionality.
    /// Used for distributing data across multiple storage instances.
    /// </summary>
    public interface IModelKey
    {
        /// <summary>
        /// Gets the hash value used for sharding and distribution logic.
        /// </summary>
        /// <returns>A hash value for determining the target shard or partition.</returns>
        uint GetHash();
    }

    /// <summary>
    /// Defines the contract for Redis value keys that provide caching functionality.
    /// Extends model key functionality with Redis-specific key generation.
    /// </summary>
    public interface IRedisValueKey : IModelKey
    {
        /// <summary>
        /// Gets the Redis key for storing this entity as a value.
        /// </summary>
        /// <returns>A Redis key string for value-based storage operations.</returns>
        RedisKey GetRedisKey();
    }

    /// <summary>
    /// Defines the contract for Redis hash keys that provide hash-based caching functionality.
    /// Extends Redis value key functionality with field-level granularity.
    /// </summary>
    public interface IRedisHashKey : IRedisValueKey
    {
        /// <summary>
        /// Gets the Redis field identifier for storing this entity within a hash.
        /// </summary>
        /// <returns>A Redis field value for hash-based storage operations.</returns>
        RedisValue GetRedisField();
    }

    /// <summary>
    /// Provides a base implementation of the <see cref="IModel"/> interface.
    /// Automatically initializes common properties with default values.
    /// </summary>
    public abstract class BaseModel : IModel
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="BaseModel"/> class.
        /// Sets default values for creation and update timestamps.
        /// </summary>
        protected BaseModel()
        { }

        /// <summary>
        /// Gets or sets the date and time when the entity was created.
        /// </summary>
        /// <value>The creation timestamp, defaulting to the current date and time.</value>
        public DateTime CreatedDate { get; set; } = DateTime.Now;

        /// <summary>
        /// Gets or sets the date and time when the entity was last updated.
        /// </summary>
        /// <value>The last modification timestamp, defaulting to the current date and time.</value>
        public DateTime UpdatedDate { get; set; } = DateTime.Now;

        /// <summary>
        /// Gets or sets a value indicating whether the entity is marked as deleted.
        /// </summary>
        /// <value>False by default, indicating the entity is active.</value>
        public bool Deleted { get; set; } = false;
    }
}