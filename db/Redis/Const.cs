namespace Db.Redis
{
    public class Const
    {
        public const string ReferenceCountKey = "count-ref";
        public static readonly TimeSpan CacheTimeToLive = TimeSpan.FromMinutes(30);
    }
}