using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents a spell key model that provides Redis hash caching functionality.
    /// Serves as a base class for spell-related models with Redis integration.
    /// </summary>
    public class SpellKey : BaseModel, IRedisHashKey
    {
        /// <summary>
        /// Gets or sets the unique identifier of the character who owns this spell.
        /// </summary>
        /// <value>The owner character's unique ID.</value>
        public required uint Owner { get; set; }

        /// <summary>
        /// Gets or sets the spell slot number where this spell is stored.
        /// </summary>
        /// <value>The slot index (0-based) in the character's spell book.</value>
        public byte Slot { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the owner ID.
        /// </summary>
        /// <returns>The owner ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Owner;

        /// <summary>
        /// Gets the Redis key for caching this character's spells.
        /// </summary>
        /// <returns>A formatted Redis key string for spell cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:spell:{Owner}";

        /// <summary>
        /// Gets the Redis field identifier for this specific spell within the hash.
        /// </summary>
        /// <returns>A formatted Redis field string based on the spell slot number.</returns>
        public RedisValue GetRedisField() => $"{Slot}";
    }

    /// <summary>
    /// Represents a complete spell model with casting information.
    /// Contains spell type, cooldown, and timing data for character spells.
    /// </summary>
    public class Spell : SpellKey, IModel
    {
        /// <summary>
        /// Gets or sets the spell's model identifier.
        /// </summary>
        /// <value>The unique model ID that defines the spell type and effects.</value>
        public required int Model { get; set; }

        /// <summary>
        /// Gets or sets the next time this spell can be cast.
        /// </summary>
        /// <value>The DateTime when the spell cooldown expires and it can be used again.</value>
        public DateTime Next { get; set; }
    }
}
