using StackExchange.Redis;

namespace Http.Model
{
    public class SystemMailUserKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public required uint MailId { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:system_mail_user:{User}";

        public RedisValue GetRedisField() => MailId.ToString();
    }

    public class SystemMailUser : SystemMailUserKey, IModel
    {
        public bool Read { get; set; }
        public DateTime? ExpireDate { get; set; }
    }
}

