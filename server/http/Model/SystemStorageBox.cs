using Fb.Model;
using StackExchange.Redis;

namespace Http.Model
{
    public class SystemStorageBoxKey : BaseModel, IRedisHashKey
    {
        public required uint Id { get; set; }
        public uint? User { get; set; }
        public uint? GetHash() => null;

        public RedisKey GetRedisKey() => User == null ? "fb:cache:system_storage_box" : $"fb:cache:system_storage_box:user:{User}";

        public RedisValue GetRedisField() => Id.ToString();
    }

    public class SystemStorageBox : SystemStorageBoxKey, IModel
    {
        public string Title { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;
        public List<Dsl> Attachments { get; set; } = new List<Dsl>();
        public DateTime? ReserveDate { get; set; }
        public DateTime? ExpiredDate { get; set; }
        public bool IncludeNewCharacters { get; set; }
        public bool Canceled { get; set; }
        public string ExternalRef { get; set; }

        public DateTime SendDate => ReserveDate ?? CreatedDate;

        public bool Active(DateTime now) => Canceled == false && SendDate <= now && (ExpiredDate == null || ExpiredDate > now);
    }
}
