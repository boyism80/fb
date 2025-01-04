namespace Http.Redis
{
    public class Const
    {
        public const string ReferenceCountKey = "count-ref";
        public const string RedisBufferKey = "exec-buffer";
        public static readonly TimeSpan CacheTimeToLive = TimeSpan.FromMinutes(30);
    }
}