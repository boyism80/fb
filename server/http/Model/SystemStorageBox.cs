using Fb.Model;
using StackExchange.Redis;

namespace Http.Model
{
    public class SystemStorageBoxKey : BaseModel, IRedisHashKey
    {
        public required uint Id { get; set; }
        public uint? GetHash() => null;

        public RedisKey GetRedisKey() => "fb:cache:system_storage_box";

        public RedisValue GetRedisField() => Id.ToString();
    }

    public class SystemStorageBox : SystemStorageBoxKey, IModel
    {
        public string Title { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;
        public List<Dsl> Attachments { get; set; } = new List<Dsl>();
        public DateTime? ExpiredDate { get; set; }
        public string ExternalRef { get; set; }
    }
}
