namespace Http.Model
{
    /// <summary>
    /// Represents the result of a character name reservation operation.
    /// Contains the outcome and associated character information for name reservation requests.
    /// </summary>
    public class ReserveNameResult
    {
        /// <summary>
        /// Gets or sets a value indicating whether the name reservation was successful.
        /// </summary>
        /// <value>True if the name was successfully reserved; otherwise, false.</value>
        public bool Result { get; set; }

        /// <summary>
        /// Gets or sets the unique identifier of the character associated with the name reservation.
        /// </summary>
        /// <value>The character's unique ID if reservation was successful; otherwise, may be 0 or invalid.</value>
        public uint Uid { get; set; }
    }

    /// <summary>
    /// Represents the result of a clan name setting operation.
    /// Contains the outcome and associated clan information for clan name change requests.
    /// </summary>
    public class ClanNameSetResult
    {
        /// <summary>
        /// Gets or sets a value indicating whether the clan name setting was successful.
        /// </summary>
        /// <value>True if the clan name was successfully set; otherwise, false.</value>
        public bool Result { get; set; }

        /// <summary>
        /// Gets or sets the unique identifier of the clan associated with the name setting operation.
        /// </summary>
        /// <value>The clan's unique ID if the operation was successful; otherwise, may be 0 or invalid.</value>
        public uint Id { get; set; }
    }
}
