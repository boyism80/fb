using Db.Reepository;
using Db.Service;
using http.Redis;
using http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;
using System.Data;

namespace Db.Model
{
    public class ItemKey : IRedisCacheKey
    {
        public required uint Owner { get; set; }
        public short Index { get; set; }
        public short Parts { get; set; }
        public short Deposited { get; set; }

        public uint GetDbKey() => Owner;
        public RedisKey GetRedisKey() => $"cache:item:{Owner}";
        public RedisValue GetRedisField() => $"{Index}-{Parts}-{Deposited}";
    }

    public class Item : ItemKey, IModel
    {
        public required uint Model { get; set; }
        public required ushort Count { get; set; } = 1;
        public required uint? Durability { get; set; }
        public required string CustomName { get; set; }
        public required bool Deleted { get; set; }

        public ItemKey Key => new ItemKey
        {
            Owner = Owner,
            Index = Index,
            Parts = Parts,
            Deposited = Deposited,
        };
    }
}
