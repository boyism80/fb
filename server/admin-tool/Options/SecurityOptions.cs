namespace AdminTool.Options
{
    /// <summary>
    /// Represents security-related configuration values for the admin tool.
    /// </summary>
    public class SecurityOptions
    {
        /// <summary>
        /// Gets or sets the shared elevation secret used to bypass role restrictions.
        /// </summary>
        public string ElevationSecret { get; set; } = string.Empty;
    }
}

