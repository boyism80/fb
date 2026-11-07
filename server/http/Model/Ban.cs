using StackExchange.Redis;

namespace Http.Model
{
    public class BanKey : BaseModel, IRedisValueKey
    {
        public required uint User { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:ban:{User}";
    }

    public class Ban : BanKey, IModel
    {
        public string Reason { get; set; }
        public DateTime? ExpireDate { get; set; }
        public Fb.Model.EnumValue.Role? ActorRole { get; set; }

        public static string OperationLockKey(uint user)
        {
            return $"fb:lock:ban-op:{user}";
        }
    }
}
