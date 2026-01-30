using Newtonsoft.Json;

namespace Http.Model
{
    /// <summary>
    /// Represents a maintenance schedule with time window and repetition pattern.
    /// </summary>
    public class MaintenanceSchedule
    {
        /// <summary>
        /// Gets or sets the unique identifier for the maintenance schedule (GUID).
        /// </summary>
        [JsonProperty("id")]
        public string Id { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the world identifier. Use 0 for all worlds.
        /// </summary>
        [JsonProperty("world")]
        public uint World { get; set; }

        /// <summary>
        /// Gets or sets the title of the maintenance schedule.
        /// </summary>
        [JsonProperty("title")]
        public string Title { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the message to display to users during maintenance.
        /// </summary>
        [JsonProperty("message")]
        public string Message { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the start time of the maintenance window (UTC).
        /// </summary>
        [JsonProperty("start_time")]
        public DateTime StartTime { get; set; }

        /// <summary>
        /// Gets or sets the end time of the maintenance window (UTC).
        /// </summary>
        [JsonProperty("end_time")]
        public DateTime EndTime { get; set; }

        /// <summary>
        /// Gets or sets the repetition type for the maintenance schedule.
        /// </summary>
        [JsonProperty("repeat_type")]
        public MaintenanceRepeatType RepeatType { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the schedule is active (not cancelled).
        /// </summary>
        [JsonProperty("is_active")]
        public bool IsActive { get; set; } = true;

        /// <summary>
        /// Gets or sets the creation timestamp (UTC).
        /// </summary>
        [JsonProperty("created_at")]
        public DateTime CreatedAt { get; set; } = DateTime.UtcNow;

        /// <summary>
        /// Gets or sets the cancellation timestamp (UTC), if cancelled.
        /// </summary>
        [JsonProperty("cancelled_at")]
        public DateTime? CancelledAt { get; set; }
    }

    /// <summary>
    /// Represents the repetition pattern for maintenance schedules.
    /// </summary>
    [JsonConverter(typeof(Newtonsoft.Json.Converters.StringEnumConverter))]
    public enum MaintenanceRepeatType
    {
        /// <summary>
        /// One-time maintenance (no repetition).
        /// </summary>
        None = 0,

        /// <summary>
        /// Repeat every day at the same time.
        /// </summary>
        Daily = 1,

        /// <summary>
        /// Repeat every week on the same day of week.
        /// </summary>
        Weekly = 2,

        /// <summary>
        /// Repeat every month on the same day of month.
        /// </summary>
        Monthly = 3,

        /// <summary>
        /// Repeat every year on the same month and day.
        /// </summary>
        Yearly = 4
    }
}
