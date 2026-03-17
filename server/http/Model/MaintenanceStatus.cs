using Newtonsoft.Json;

namespace Http.Model
{
    public class MaintenanceStatus
    {
        [JsonProperty("is_active")]
        public bool IsActive { get; set; }

        [JsonProperty("next_check_time")]
        public DateTime? NextCheckTime { get; set; }

        [JsonProperty("current_maintenance")]
        public MaintenanceInfo CurrentMaintenance { get; set; }
    }
}