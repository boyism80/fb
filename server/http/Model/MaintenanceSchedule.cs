using Newtonsoft.Json;

namespace Http.Model
{
    public static class MaintenanceLabel
    {
        public const string All = "all";
        public const string Cross = "cross";

        private const string WorldPrefix = "world:";

        public static string World(uint world) => $"{WorldPrefix}{world}";

        public static bool TryParseWorld(string label, out uint world)
        {
            world = 0;
            return label != null && label.StartsWith(WorldPrefix) && uint.TryParse(label[WorldPrefix.Length..], out world);
        }

        public static bool Matches(IEnumerable<string> labels, string label) =>
            labels.Contains(All) || labels.Contains(label);

        public static string Format(IEnumerable<string> labels) =>
            string.Join(", ", labels.Select(x => x switch
            {
                All => "전체 서버",
                Cross => "교차 서버",
                _ => TryParseWorld(x, out var world) ? $"World {world}" : x
            }));
    }

    public class MaintenanceSchedule
    {
        [JsonProperty("id")]
        public string Id { get; set; } = string.Empty;

        [JsonProperty("labels")]
        public List<string> Labels { get; set; } = new();

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

        [JsonProperty("finish_matches")]
        public bool FinishMatches { get; set; }

        [JsonProperty("notice_minutes")]
        public List<int> NoticeMinutes { get; set; } = new();

        // A recurring schedule edited during a window keeps that window's times here; later windows use the template.
        [JsonProperty("occurrence")]
        public MaintenanceOccurrence Occurrence { get; set; }

        [JsonProperty("revision")]
        public int Revision { get; set; }

        [JsonProperty("is_active")]
        public bool IsActive { get; set; } = true;

        [JsonProperty("created_at")]
        public DateTime CreatedAt { get; set; } = DateTime.UtcNow;

        [JsonProperty("cancelled_at")]
        public DateTime? CancelledAt { get; set; }
    }

    public class MaintenanceOccurrence
    {
        [JsonProperty("start")]
        public DateTime Start { get; set; }

        [JsonProperty("end")]
        public DateTime End { get; set; }
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
