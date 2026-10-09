namespace Http.Redis.Key
{
    public class HeartBeatKey : IRedisKey
    {
        public const string Index = "fb:heart-beat-index";
        public const string Prefix = "fb:heart-beat:";

        public uint? World { get; set; }
        public fb.protocol.@internal.Service Service { get; set; }
        public byte Id { get; set; } = 0xFF;

        public string Member => World.HasValue
            ? $"{World.Value}:{Service}:{Id}"
            : $"cross:{Service}:{Id}";

        public string Key => Prefix + Member;
    }
}
