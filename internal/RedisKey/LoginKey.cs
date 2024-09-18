namespace Internal.RedisKey
{
    public class LoginKey : IRedisKey
    {
        public string Key => $"user:login";
    }
}
