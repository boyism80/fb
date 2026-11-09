using Fb.Model.EnumValue;
using StackExchange.Redis;

namespace Http.Model
{
    public class ClanMemberKey : BaseModel, IRedisHashKey
    {
        public required uint Clan { get; set; }
        public uint User { get; set; }
        public uint? GetHash() => Clan;

        public RedisKey GetRedisKey() => $"fb:cache:clan-member:{Clan}";

        public RedisValue GetRedisField() => $"{User}";
    }

    public class ClanMember : ClanMemberKey, IModel
    {
        public uint Role { get; set; }

        public void Command(ClanMember target, uint minimumPrivilege)
        {
            if (Role < minimumPrivilege)
            {
                throw new LogicException(ErrorCode.ClanNoPrivilege);
            }

            if (target.Role >= Role)
            {
                throw new LogicException(ErrorCode.ClanNoPrivilege);
            }
        }
    }
}
