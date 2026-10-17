using StackExchange.Redis;

namespace Http.Model
{
    public class ClanKey : BaseModel, IRedisValueKey
    {
        public required uint Id { get; set; }
        public uint? GetHash() => Id;

        public RedisKey GetRedisKey() => $"fb:cache:clan:{Id}";
    }

    public class ClanMoneyRequest
    {
        public ulong Id { get; set; }
        public bool Applied { get; set; }
        public DateTime Date { get; set; }
    }

    public class Clan : ClanKey, IModel
    {
        public string Name { get; set; }
        public string Title { get; set; }
        public ulong Money { get; set; }
        public ulong Revision { get; set; }

        // Kept in the clan record so a money change and its request state are written by the same SET.
        public List<ClanMoneyRequest> MoneyRequests { get; set; } = new List<ClanMoneyRequest>();

        // Older requests move to clan_money_request_archive, which is never pruned, so the clan record stays small.
        public static readonly TimeSpan MoneyRequestRetention = TimeSpan.FromDays(1);

        public static string DistributedLockKey(uint id)
        {
            return $"fb:lock:clan:{id}";
        }
    }
}
