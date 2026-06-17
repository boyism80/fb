namespace Http.Redis
{
    public class Const
    {
        public const string ReferenceCountKey = "fb:cref";
        public const string RedisBufferKey = "fb:exec-buffer";
        public static readonly TimeSpan CacheTimeToLive = TimeSpan.FromMinutes(30);
    }
}
