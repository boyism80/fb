using Newtonsoft.Json;

namespace Http.Model
{
    /// <summary>
    /// Represents the cached maintenance status for a world.
    /// </summary>
    public class MaintenanceStatus
    {
        /// <summary>
        /// Gets or sets a value indicating whether maintenance is currently active.
        /// </summary>
        [JsonProperty("is_active")]
        public bool IsActive { get; set; }

        /// <summary>
        /// Gets or sets the next time to check maintenance status (UTC).
        /// Used for cache invalidation.
        /// </summary>
        [JsonProperty("next_check_time")]
        public DateTime? NextCheckTime { get; set; }

        /// <summary>
        /// Gets or sets the current maintenance information, if active.
        /// </summary>
        [JsonProperty("current_maintenance")]
        public MaintenanceInfo CurrentMaintenance { get; set; }
    }
}
