namespace Http.Model.Redis
{
    /// <summary>
    /// Represents a user session model for Redis storage.
    /// Contains session information linking a user to their current host server.
    /// </summary>
    public class Session
    {
        /// <summary>
        /// Gets or sets the unique identifier of the user associated with this session.
        /// </summary>
        /// <value>The user's unique character ID.</value>
        public uint Uid { get; set; }

        /// <summary>
        /// Gets or sets the host server identifier where the user is currently connected.
        /// </summary>
        /// <value>The host server ID managing this user's session.</value>
        public uint Host { get; set; }
    }
}
