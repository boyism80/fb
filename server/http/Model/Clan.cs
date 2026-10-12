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
        public bool Released { get; set; }
        public DateTime Date { get; set; }
    }

    public class Clan : ClanKey, IModel
    {
        public string Name { get; set; }
        public string Title { get; set; }
        public ulong Money { get; set; }

        // Kept in the clan record so a money change and its request state are written by the same SET.
        public List<ClanMoneyRequest> MoneyRequests { get; set; } = new List<ClanMoneyRequest>();

        // A request is dropped only after the game releases it, so a late request still finds its outcome.
        public static readonly TimeSpan MoneyRequestRetention = TimeSpan.FromDays(1);

        public void AddMoneyRequest(ulong id, bool applied, DateTime now)
        {
            MoneyRequests.RemoveAll(x => x.Released && now - x.Date > MoneyRequestRetention);
            MoneyRequests.Add(new ClanMoneyRequest { Id = id, Applied = applied, Date = now });
        }

        public static string DistributedLockKey(uint id)
        {
            return $"fb:lock:clan:{id}";
        }
    }
}
