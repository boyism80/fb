using StackExchange.Redis;

namespace Http.Model
{
    public class FriendRevisionKey : BaseModel, IRedisValueKey
    {
        public required uint User { get; set; }
        public uint? GetHash() => User;

        public RedisKey GetRedisKey() => $"fb:cache:friend_revision:{User}";
    }

    public class FriendRevision : FriendRevisionKey, IModel
    {
        public ulong Revision { get; set; }
    }
}
