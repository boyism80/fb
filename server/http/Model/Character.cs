using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents a character key model that provides Redis caching functionality.
    /// Serves as a base class for character-related models with Redis integration.
    /// </summary>
    public class CharacterKey : BaseModel, IRedisValueKey
    {
        /// <summary>
        /// Gets or sets the unique identifier for the character.
        /// </summary>
        /// <value>The character's unique ID used for database and cache operations.</value>
        public required uint Id { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the character ID.
        /// </summary>
        /// <returns>The character ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Id;

        /// <summary>
        /// Gets the Redis key for caching this character.
        /// </summary>
        /// <returns>A formatted Redis key string for character cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:user:{Id}";
    }

    /// <summary>
    /// Represents a complete character model with all game-related properties.
    /// Contains character attributes, appearance, stats, and equipment information.
    /// </summary>
    public class Character : CharacterKey, IModel
    {
        /// <summary>
        /// Gets or sets the character's name.
        /// </summary>
        /// <value>The display name of the character.</value>
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the character's password.
        /// </summary>
        /// <value>The encrypted password for character authentication.</value>
        public string Pw { get; set; }

        /// <summary>
        /// Gets or sets the character's birth date as a timestamp.
        /// </summary>
        /// <value>The birth date timestamp, or null if not set.</value>
        public uint? Birth { get; set; }

        /// <summary>
        /// Gets or sets the character's role in the game.
        /// </summary>
        /// <value>The role enum value defining character permissions and capabilities.</value>
        public Fb.Model.EnumValue.Role Role { get; set; }

        /// <summary>
        /// Gets or sets the character's visual appearance ID.
        /// </summary>
        /// <value>The sprite/appearance identifier for character rendering.</value>
        public ushort Look { get; set; }

        /// <summary>
        /// Gets or sets the character's color scheme.
        /// </summary>
        /// <value>The color value for character appearance customization.</value>
        public byte Color { get; set; }

        /// <summary>
        /// Gets or sets the character's gender.
        /// </summary>
        /// <value>The gender identifier (0 = male, 1 = female).</value>
        public byte Sex { get; set; }

        /// <summary>
        /// Gets or sets the character's nation affiliation.
        /// </summary>
        /// <value>The nation identifier for character's political alignment.</value>
        public byte Nation { get; set; }

        /// <summary>
        /// Gets or sets the character's creature type.
        /// </summary>
        /// <value>The creature type identifier, or null if not applicable.</value>
        public byte? Creature { get; set; }

        /// <summary>
        /// Gets or sets the ID of the map where the character is located.
        /// </summary>
        /// <value>The map identifier for character's current location.</value>
        public uint Map { get; set; }

        /// <summary>
        /// Gets or sets the character's X coordinate position.
        /// </summary>
        /// <value>The horizontal position on the current map.</value>
        public ushort PositionX { get; set; }

        /// <summary>
        /// Gets or sets the character's Y coordinate position.
        /// </summary>
        /// <value>The vertical position on the current map.</value>
        public ushort PositionY { get; set; }

        /// <summary>
        /// Gets or sets the character's facing direction.
        /// </summary>
        /// <value>The direction identifier (0-7 representing 8 directions).</value>
        public byte Direction { get; set; }

        /// <summary>
        /// Gets or sets the character's current state.
        /// </summary>
        /// <value>The state identifier (normal, dead, sleeping, etc.).</value>
        public byte State { get; set; }

        /// <summary>
        /// Gets or sets the character's class.
        /// </summary>
        /// <value>The class identifier (warrior, rogue, wizard, priest).</value>
        public byte Class { get; set; }

        /// <summary>
        /// Gets or sets the character's promotion level.
        /// </summary>
        /// <value>The promotion level (0-4) indicating character advancement.</value>
        public byte Promotion { get; set; }

        /// <summary>
        /// Gets or sets the character's current level.
        /// </summary>
        /// <value>The character level (1-99).</value>
        public byte Level { get; set; }

        /// <summary>
        /// Gets or sets the character's experience points.
        /// </summary>
        /// <value>The current experience points accumulated by the character.</value>
        public uint Exp { get; set; }

        /// <summary>
        /// Gets or sets the character's current money amount.
        /// </summary>
        /// <value>The amount of money in character's inventory.</value>
        public uint Money { get; set; }

        /// <summary>
        /// Gets or sets the character's deposited money amount.
        /// </summary>
        /// <value>The amount of money stored in bank or storage.</value>
        public uint DepositedMoney { get; set; }

        /// <summary>
        /// Gets or sets the character's disguise appearance.
        /// </summary>
        /// <value>The disguise appearance ID, or null if not disguised.</value>
        public ushort? Disguise { get; set; }

        /// <summary>
        /// Gets or sets the character's current health points.
        /// </summary>
        /// <value>The current HP value.</value>
        public uint Hp { get; set; }

        /// <summary>
        /// Gets or sets the character's base health points.
        /// </summary>
        /// <value>The base HP value without equipment or buff modifiers.</value>
        public uint BaseHp { get; set; }

        /// <summary>
        /// Gets or sets the character's additional health points from equipment/buffs.
        /// </summary>
        /// <value>The additional HP value from external sources.</value>
        public uint AdditionalHp { get; set; }

        /// <summary>
        /// Gets or sets the character's current mana points.
        /// </summary>
        /// <value>The current MP value.</value>
        public uint Mp { get; set; }

        /// <summary>
        /// Gets or sets the character's base mana points.
        /// </summary>
        /// <value>The base MP value without equipment or buff modifiers.</value>
        public uint BaseMp { get; set; }

        /// <summary>
        /// Gets or sets the character's additional mana points from equipment/buffs.
        /// </summary>
        /// <value>The additional MP value from external sources.</value>
        public uint AdditionalMp { get; set; }

        /// <summary>
        /// Gets or sets the character's weapon color customization.
        /// </summary>
        /// <value>The weapon color override, or null for default color.</value>
        public byte? WeaponColor { get; set; }

        /// <summary>
        /// Gets or sets the character's helmet color customization.
        /// </summary>
        /// <value>The helmet color override, or null for default color.</value>
        public byte? HelmetColor { get; set; }

        /// <summary>
        /// Gets or sets the character's armor color customization.
        /// </summary>
        /// <value>The armor color override, or null for default color.</value>
        public byte? ArmorColor { get; set; }

        /// <summary>
        /// Gets or sets the character's shield color customization.
        /// </summary>
        /// <value>The shield color override, or null for default color.</value>
        public byte? ShieldColor { get; set; }

        /// <summary>
        /// Gets or sets the character's left ring color customization.
        /// </summary>
        /// <value>The left ring color override, or null for default color.</value>
        public byte? RingLeftColor { get; set; }

        /// <summary>
        /// Gets or sets the character's right ring color customization.
        /// </summary>
        /// <value>The right ring color override, or null for default color.</value>
        public byte? RingRightColor { get; set; }

        /// <summary>
        /// Gets or sets the character's top auxiliary equipment color customization.
        /// </summary>
        /// <value>The top auxiliary color override, or null for default color.</value>
        public byte? AuxTopColor { get; set; }

        /// <summary>
        /// Gets or sets the character's bottom auxiliary equipment color customization.
        /// </summary>
        /// <value>The bottom auxiliary color override, or null for default color.</value>
        public byte? AuxBotColor { get; set; }

        /// <summary>
        /// Gets or sets the list of active buffs applied to the character.
        /// </summary>
        /// <value>A collection of buff effects currently affecting the character.</value>
        public List<Buff> Buffs { get; set; } = new List<Buff>();

        /// <summary>
        /// Gets or sets the character's title or rank.
        /// </summary>
        /// <value>The display title shown with the character's name.</value>
        public string Title { get; set; } = string.Empty;
    }
}
