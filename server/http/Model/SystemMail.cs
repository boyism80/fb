using StackExchange.Redis;

namespace Http.Model
{
    public class SystemMailKey : BaseModel, IRedisHashKey
    {
        public required uint Id { get; set; }
        public uint? GetHash() => null;

        public RedisKey GetRedisKey() => "fb:cache:system_mail";

        public RedisValue GetRedisField() => Id.ToString();
    }

    public class SystemMail : SystemMailKey, IModel
    {
        public uint Sender { get; set; }
        public string Title { get; set; } = string.Empty;
        public string Contents { get; set; } = string.Empty;
        public DateTime? ReserveDate { get; set; }
        public DateTime? ExpireDate { get; set; }
        public bool IncludeNewCharacters { get; set; }
        public bool Canceled { get; set; }

        public DateTime SendDate => ReserveDate ?? CreatedDate;

        public bool Active(DateTime now) => Canceled == false && SendDate <= now && (ExpireDate == null || ExpireDate > now);
    }
}
