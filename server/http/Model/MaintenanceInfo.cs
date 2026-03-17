using Newtonsoft.Json;

namespace Http.Model
{
    public class MaintenanceInfo
    {
        [JsonProperty("is_active")]
        public bool IsActive { get; set; }

        [JsonProperty("message")]
        public string Message { get; set; } = string.Empty;

        [JsonProperty("start_time")]
        public DateTime StartTime { get; set; }

        [JsonProperty("end_time")]
        public DateTime EndTime { get; set; }

        [JsonProperty("schedule_id")]
        public string ScheduleId { get; set; } = string.Empty;
    }
}