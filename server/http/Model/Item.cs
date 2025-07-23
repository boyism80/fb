using StackExchange.Redis;
using System.Text.Json.Serialization;

namespace Http.Model
{
    /// <summary>
    /// Represents an item key model that provides Redis hash caching functionality.
    /// Serves as a base class for item-related models with Redis integration.
    /// </summary>
    public class ItemKey : BaseModel, IRedisHashKey
    {
        /// <summary>
        /// Gets or sets the unique identifier of the character who owns this item.
        /// </summary>
        /// <value>The owner character's unique ID.</value>
        public required uint Owner { get; set; }

        /// <summary>
        /// Gets or sets the index position of the item in the inventory.
        /// </summary>
        /// <value>The inventory slot index where the item is located.</value>
        public short Index { get; set; }

        /// <summary>
        /// Gets or sets the equipment parts identifier for the item.
        /// </summary>
        /// <value>The parts identifier indicating which equipment slot this item belongs to.</value>
        public short Parts { get; set; }

        /// <summary>
        /// Gets or sets the storage type identifier for the item.
        /// </summary>
        /// <value>The storage type (inventory, bank, etc.) where the item is stored.</value>
        public short Stored { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the owner ID.
        /// </summary>
        /// <returns>The owner ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Owner;

        /// <summary>
        /// Gets the Redis key for caching this item's owner data.
        /// </summary>
        /// <returns>A formatted Redis key string for item cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:item:{Owner}";

        /// <summary>
        /// Gets the Redis field identifier for this specific item within the hash.
        /// </summary>
        /// <returns>A formatted Redis field string combining index, parts, and storage information.</returns>
        public RedisValue GetRedisField() => $"{Index}-{Parts}-{Stored}";
    }

    /// <summary>
    /// Represents a complete item model with all game-related properties.
    /// Contains item attributes, durability, count, and customization information.
    /// </summary>
    public class Item : ItemKey, IModel
    {
        /// <summary>
        /// Gets or sets the item's model identifier.
        /// </summary>
        /// <value>The unique model ID that defines the item type and appearance.</value>
        public required uint Model { get; set; }

        /// <summary>
        /// Gets or sets the quantity of this item stack.
        /// </summary>
        /// <value>The number of items in this stack (default is 1).</value>
        public required ushort Count { get; set; } = 1;

        /// <summary>
        /// Gets or sets the item's current durability value.
        /// </summary>
        /// <value>The durability points remaining, or null if the item doesn't have durability.</value>
        public required uint? Durability { get; set; }

        /// <summary>
        /// Gets or sets the custom name assigned to this item.
        /// </summary>
        /// <value>The player-assigned custom name for the item.</value>
        public required string CustomName { get; set; }

        /// <summary>
        /// Gets the item key information for this item.
        /// </summary>
        /// <value>An ItemKey instance containing the positioning and ownership information.</value>
        [JsonIgnore]
        public ItemKey Key => new ItemKey
        {
            Owner = Owner,
            Index = Index,
            Parts = Parts,
            Stored = Stored,
        };
    }
}
