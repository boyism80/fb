using fb.protocol.inter;

namespace Internal.RedisKey
{
    public class HeartBeatKey : IRedisKey
    {
        public Service Service { get; set; }
        public byte Group { get; set; } = 0xFF;

        public string Key => $"heart-beat:{Service}:{Group}";
    }
}
