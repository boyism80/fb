namespace Http.Model
{
    /// <summary>
    /// Represents a buff effect model containing buff type and duration information.
    /// Used to track temporary status effects applied to characters in the game.
    /// </summary>
    public class Buff
    {
        /// <summary>
        /// Gets or sets the unique identifier of the buff model/type.
        /// </summary>
        /// <value>The buff model ID that defines the type of effect and its properties.</value>
        public uint Model { get; set; }

        /// <summary>
        /// Gets or sets the remaining time for this buff effect.
        /// </summary>
        /// <value>The time value representing how long the buff effect will last (in game time units).</value>
        public uint Time { get; set; }
    }
}
