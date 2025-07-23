using StackExchange.Redis;

namespace Http.Model
{
    /// <summary>
    /// Represents an option key model that provides Redis value caching functionality.
    /// Serves as a base class for user option-related models with Redis integration.
    /// </summary>
    public class OptionKey : BaseModel, IRedisValueKey
    {
        /// <summary>
        /// Gets or sets the unique identifier of the user who owns these options.
        /// </summary>
        /// <value>The user's unique ID associated with these options.</value>
        public required uint Uid { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the user ID.
        /// </summary>
        /// <returns>The user ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Uid;

        /// <summary>
        /// Gets the Redis key for caching this user's options.
        /// </summary>
        /// <returns>A formatted Redis key string for option cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:option:{Uid}";
    }

    /// <summary>
    /// Represents a complete user options model with all game preference settings.
    /// Contains boolean flags for various game features and user interface preferences.
    /// </summary>
    public class Option : OptionKey, IModel
    {
        /// <summary>
        /// Gets or sets a value indicating whether whisper messages are enabled.
        /// </summary>
        /// <value>True if whisper messages are allowed; otherwise, false. Default is true.</value>
        public bool Whisper { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether group messages are enabled.
        /// </summary>
        /// <value>True if group messages are allowed; otherwise, false. Default is true.</value>
        public bool Group { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether roar messages are enabled.
        /// </summary>
        /// <value>True if roar messages are allowed; otherwise, false. Default is true.</value>
        public bool Roar { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether world roar messages are enabled.
        /// </summary>
        /// <value>True if world-wide roar messages are allowed; otherwise, false. Default is true.</value>
        public bool RoarWorlds { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether magic visual effects are enabled.
        /// </summary>
        /// <value>True if magic effects are displayed; otherwise, false. Default is true.</value>
        public bool MagicEffect { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether weather visual effects are enabled.
        /// </summary>
        /// <value>True if weather effects are displayed; otherwise, false. Default is true.</value>
        public bool WeatherEffect { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether fixed movement mode is enabled.
        /// </summary>
        /// <value>True if fixed movement is enabled; otherwise, false. Default is true.</value>
        public bool FixedMove { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether trading is enabled.
        /// </summary>
        /// <value>True if trading with other players is allowed; otherwise, false. Default is true.</value>
        public bool Trade { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether fast movement mode is enabled.
        /// </summary>
        /// <value>True if fast movement is enabled; otherwise, false. Default is true.</value>
        public bool FastMove { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether effect sounds are enabled.
        /// </summary>
        /// <value>True if sound effects are played; otherwise, false. Default is true.</value>
        public bool EffectSound { get; set; } = true;

        /// <summary>
        /// Gets or sets a value indicating whether PK (Player Killing) protection is enabled.
        /// </summary>
        /// <value>True if PK protection is active; otherwise, false. Default is true.</value>
        public bool PkProtect { get; set; } = true;
    }
}