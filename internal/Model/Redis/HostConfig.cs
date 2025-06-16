namespace Http.Model.Redis
{
    /// <summary>
    /// Represents a host server configuration model for Redis storage.
    /// Contains network connection information for game servers.
    /// </summary>
    public class HostConfig
    {
        /// <summary>
        /// Gets or sets the display name of the host server.
        /// </summary>
        /// <value>The human-readable name identifier for the server.</value>
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the IP address of the host server.
        /// </summary>
        /// <value>The network IP address where the server is accessible.</value>
        public string IP { get; set; }

        /// <summary>
        /// Gets or sets the port number of the host server.
        /// </summary>
        /// <value>The network port number where the server is listening for connections.</value>
        public ushort Port { get; set; }
    }
}
