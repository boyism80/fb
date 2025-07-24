namespace Http.Model
{
    /// <summary>
    /// Represents a character name model containing basic character identification information.
    /// Used for character name lookups and display purposes throughout the application.
    /// </summary>
    public class CharacterName
    {
        /// <summary>
        /// Gets or sets the unique identifier of the character.
        /// </summary>
        /// <value>The character's unique ID used for database operations and references.</value>
        public uint Id { get; set; }

        /// <summary>
        /// Gets or sets the display name of the character.
        /// </summary>
        /// <value>The character's name as it appears in the game and user interfaces.</value>
        public string Name { get; set; }
    }
}
