namespace Http.Redis
{
    public class Const
    {
        public const string ReferenceCountKey = "fb:count-ref";
        public const string RedisBufferKey = "fb:exec-buffer";
        public static readonly TimeSpan CacheTimeToLive = TimeSpan.FromMinutes(30);
    }
}