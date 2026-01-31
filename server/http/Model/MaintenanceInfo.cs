using Newtonsoft.Json;

namespace Http.Model
{
    /// <summary>
    /// Represents the current maintenance status information for a world.
    /// </summary>
    public class MaintenanceInfo
    {
        /// <summary>
        /// Gets or sets a value indicating whether maintenance is currently active.
        /// </summary>
        [JsonProperty("is_active")]
        public bool IsActive { get; set; }

        /// <summary>
        /// Gets or sets the maintenance message to display to users.
        /// </summary>
        [JsonProperty("message")]
        public string Message { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the start time of the current maintenance window (UTC).
        /// </summary>
        [JsonProperty("start_time")]
        public DateTime StartTime { get; set; }

        /// <summary>
        /// Gets or sets the end time of the current maintenance window (UTC).
        /// </summary>
        [JsonProperty("end_time")]
        public DateTime EndTime { get; set; }

        /// <summary>
        /// Gets or sets the schedule ID that triggered this maintenance.
        /// </summary>
        [JsonProperty("schedule_id")]
        public string ScheduleId { get; set; } = string.Empty;
    }
}
