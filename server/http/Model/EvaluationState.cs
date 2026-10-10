using Newtonsoft.Json;

namespace Http.Model
{
    public class EvaluationState
    {
        [JsonProperty("playtime")]
        public uint Playtime { get; set; }

        // Target character id -> evaluated time; the game drops entries older than the cooldown.
        [JsonProperty("targets")]
        public Dictionary<uint, string> Targets { get; set; } = new Dictionary<uint, string>();
    }
}
