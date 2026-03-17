using Newtonsoft.Json;

namespace Http.Model
{
    public class MaintenanceSchedule
    {
        [JsonProperty("id")]
        public string Id { get; set; } = string.Empty;

        [JsonProperty("world")]
        public uint World { get; set; }

        [JsonProperty("title")]
        public string Title { get; set; } = string.Empty;

        [JsonProperty("message")]
        public string Message { get; set; } = string.Empty;

        [JsonProperty("start_time")]
        public DateTime StartTime { get; set; }

        [JsonProperty("end_time")]
        public DateTime EndTime { get; set; }

        [JsonProperty("repeat_type")]
        public MaintenanceRepeatType RepeatType { get; set; }

        [JsonProperty("is_active")]
        public bool IsActive { get; set; } = true;

        [JsonProperty("created_at")]
        public DateTime CreatedAt { get; set; } = DateTime.UtcNow;

        [JsonProperty("cancelled_at")]
        public DateTime? CancelledAt { get; set; }
    }

    [JsonConverter(typeof(Newtonsoft.Json.Converters.StringEnumConverter))]
    public enum MaintenanceRepeatType
    {
        None = 0,

        Daily = 1,

        Weekly = 2,

        Monthly = 3,

        Yearly = 4
    }
}