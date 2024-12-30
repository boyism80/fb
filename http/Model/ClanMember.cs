using StackExchange.Redis;
using System.Text.Json.Serialization;

namespace Http.Model
{
    public class ClanMemberKey : BaseModel, IRedisHashKey
    {
        public required uint Clan { get; set; }
        public uint User { get; set; }

        public uint GetHash() => Clan;
        public RedisKey GetRedisKey() => $"cache:clan-member:{Clan}";
        public RedisValue GetRedisField() => $"{User}";
    }

    public class ClanMember : ClanMemberKey, IModel
    {
        public uint Position { get; set; }
        [JsonIgnore]
        public ClanMemberKey Key => new ClanMemberKey
        {
            Clan = Clan,
            User = User,
        };
    }
}
