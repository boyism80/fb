using Newtonsoft.Json;

namespace Http.Model
{
    public class Mimicry
    {
        [JsonProperty("gender")]
        public byte Gender { get; set; }

        [JsonProperty("state")]
        public byte State { get; set; }

        [JsonProperty("hair")]
        public ushort Hair { get; set; }

        [JsonProperty("hair_color")]
        public byte HairColor { get; set; }

        [JsonProperty("weapon")]
        public ushort? Weapon { get; set; }

        [JsonProperty("weapon_color")]
        public byte? WeaponColor { get; set; }

        [JsonProperty("armor")]
        public byte? Armor { get; set; }

        [JsonProperty("armor_color")]
        public byte? ArmorColor { get; set; }

        [JsonProperty("shield")]
        public byte? Shield { get; set; }

        [JsonProperty("shield_color")]
        public byte? ShieldColor { get; set; }

        [JsonProperty("disguise")]
        public ushort? Disguise { get; set; }
    }
}
