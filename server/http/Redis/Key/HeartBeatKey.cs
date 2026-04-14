namespace Http.Redis.Key
{
    public class HeartBeatKey : IRedisKey
    {
        public uint World { get; set; }
        public fb.protocol._internal.Service Service { get; set; }
        public byte Id { get; set; } = 0xFF;

        public string Key => $"fb:heart-beat:{World}:{Service}:{Id}";
    }
}
