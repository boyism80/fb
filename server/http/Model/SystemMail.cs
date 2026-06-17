using StackExchange.Redis;

namespace Http.Model
{
    public class SystemMailKey : BaseModel, IRedisHashKey
    {
        public required uint Id { get; set; }
        public uint? GetHash() => 0;

        public RedisKey GetRedisKey() => "fb:cache:system_mail";

        public RedisValue GetRedisField() => Id.ToString();
    }

    public class SystemMail : SystemMailKey, IModel
    {
        public uint Sender { get; set; }
        public string Title { get; set; } = string.Empty;
        public string Contents { get; set; } = string.Empty;
        public DateTime? ExpireDate { get; set; }
    }
}
