using StackExchange.Redis;

namespace Http.Model
{
    public class ClanKey : BaseModel, IRedisValueKey
    {
        public required uint Id { get; set; }

        public uint GetDbKey() => Id;
        public RedisKey GetRedisKey() => $"cache:clan:{Id}";
    }

    public class Clan : ClanKey, IModel
    {
        public uint Master { get; set; }
        public string Name { get; set; }
        public string Title { get; set; }
    }
}
