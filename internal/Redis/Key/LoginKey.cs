namespace Internal.Redis.Key
{
    public class LoginKey : IRedisKey
    {
        public string Key => $"user:login";
    }
}
