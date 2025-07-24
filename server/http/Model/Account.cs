namespace Http.Model
{
    /// <summary>
    /// Represents a user account model containing basic account information.
    /// Used for account authentication and initial character spawn location data.
    /// </summary>
    public class Account
    {
        /// <summary>
        /// Gets or sets the unique identifier for this account.
        /// </summary>
        /// <value>The unique account ID used for database operations and authentication.</value>
        public uint Id { get; set; }

        /// <summary>
        /// Gets or sets the password for this account.
        /// </summary>
        /// <value>The encrypted password string used for account authentication.</value>
        public string Pw { get; set; }

        /// <summary>
        /// Gets or sets the default map identifier for this account.
        /// </summary>
        /// <value>The map ID where characters from this account will initially spawn or return to.</value>
        public uint Map { get; set; }
    }
}
