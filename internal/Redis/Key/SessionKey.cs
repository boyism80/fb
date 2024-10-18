namespace Internal.Redis.Key
{
    public class SessionKey : IRedisKey
    {
        public string Key => $"user:session";
    }
}
