namespace Http.Redis.Key
{
    public class HeartBeatKey : IRedisKey
    {
        public fb.protocol._internal.ProcessRole Role { get; set; } = fb.protocol._internal.ProcessRole.Home;
        public uint World { get; set; }
        public fb.protocol._internal.Service Service { get; set; }
        public byte Id { get; set; } = 0xFF;

        public string Key => Role == fb.protocol._internal.ProcessRole.Cross
            ? $"fb:heart-beat:cross:{Service}:{Id}"
            : $"fb:heart-beat:{World}:{Service}:{Id}";
    }
}
