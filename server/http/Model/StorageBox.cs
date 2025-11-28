using System.Collections.Generic;
using Fb.Model;
using StackExchange.Redis;

namespace Http.Model
{
    public class StorageBoxKey : BaseModel, IRedisHashKey
    {
        public required uint User { get; set; }
        public uint Id { get; set; }
        public uint GetHash() => User;

        public RedisKey GetRedisKey() => $"cache:storage:box:{User}";

        public RedisValue GetRedisField() => Id.ToString();
    }

    public class StorageBox : StorageBoxKey, IModel
    {
        public string Title { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;
        public List<Dsl> Attachments { get; set; } = new List<Dsl>();
        public bool Received { get; set; }
        public DateTime? ExpiredDate { get; set; }
    }
}

