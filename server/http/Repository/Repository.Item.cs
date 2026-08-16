using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ItemRepository : RedisHashRepository<Item, ItemKey>
    {
        public ItemRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<Item> Get(uint world, uint owner, short index, short parts, short stored)
        {
            return await base.Get(world, new ItemKey
            {
                Owner = owner,
                Index = index,
                Parts = parts,
                Stored = stored
            });
        }

        public async Task<IEnumerable<Item>> Get(uint world, uint owner)
        {
            return await base.GetAll(world, new ItemKey
            {
                Owner = owner
            });
        }

        protected override string OnSelect(ItemKey key)
        {
            return $""""
                SELECT * FROM `item` WHERE 
                `owner` = {key.Owner} AND
                `index` = {key.Index} AND
                `parts` = {key.Parts} AND
                `stored` = {key.Stored} AND
                `deleted` = 0
                LIMIT 1;
                """";
        }

        protected override string OnSelectBulk(ItemKey key)
        {
            return $"""
                SELECT * FROM `item` WHERE
                `owner` = {key.Owner} AND `deleted` = 0;
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<ItemKey> keys)
        {
            return $"SELECT * FROM `item` WHERE `owner` IN ({string.Join(",", keys.Select(k => k.Owner))}) AND `deleted` = 0;";
        }

        protected override ItemKey GetKeyFromRow(Item row)
        {
            return new ItemKey { Owner = row.Owner };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<Item>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<Item>>();

            var keys = ownerIds.Distinct().Select(oid => new ItemKey { Owner = oid }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.Owner, _ => (IList<Item>)new List<Item>());
            foreach (var item in list)
            {
                dict[item.Owner].Add(item);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<Item>)kv.Value);
        }

        protected override string OnUpsert(Item value)
        {
            var sql = $"""
                INSERT INTO `item` (
                    `owner`,
                    `index`,
                    `parts`,
                    `stored`,
                    `model`,
                    `count`,
                    `durability`,
                    `custom_name`,
                    `expire_time`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Owner.Escape()},
                    {value.Index.Escape()},
                    {value.Parts.Escape()},
                    {value.Stored.Escape()},
                    {value.Model.Escape()},
                    {value.Count.Escape()},
                    {value.Durability.Escape()},
                    {value.CustomName.Escape()},
                    {value.ExpireTime.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `model`=VALUES(`model`), 
                    `count`=VALUES(`count`), 
                    `durability`=VALUES(`durability`),
                    `custom_name`=VALUES(`custom_name`),
                    `expire_time`=VALUES(`expire_time`),
                    `deleted`=0,
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        protected override string OnUpsert(Item[] values)
        {
            var args = values.Select(item =>
            {
                return $"""
                        ({item.Owner.Escape()},
                         {item.Index.Escape()},
                         {item.Parts.Escape()},
                         {item.Stored.Escape()},
                         {item.Model.Escape()},
                         {item.Count.Escape()},
                         {item.Durability.Escape()},
                         {item.CustomName.Escape()},
                         {item.ExpireTime.Escape()},
                         0,
                         {item.CreatedDate.Escape()},
                         {item.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO item (
                        `owner`,
                        `index`,
                        `parts`,
                        `stored`,
                        `model`,
                        `count`,
                        `durability`,
                        `custom_name`,
                        `expire_time`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `model`=VALUES(`model`),
                        `count`=VALUES(`count`),
                        `durability`=VALUES(`durability`),
                        `custom_name`=VALUES(`custom_name`),
                        `expire_time`=VALUES(`expire_time`),
                        `deleted`=0,
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }

        protected override string OnDelete(ItemKey key)
        {
            return $"""
                UPDATE `item` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `owner` = {key.Owner.Escape()} AND `index` = {key.Index.Escape()} AND `parts` = {key.Parts.Escape()} AND `stored` = {key.Stored.Escape()} AND `deleted` = 0;
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<ItemKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var conditions = keys.Select(k =>
                $"(`owner` = {k.Owner.Escape()} AND `index` = {k.Index.Escape()} AND `parts` = {k.Parts.Escape()} AND `stored` = {k.Stored.Escape()})");

            return $"""
                UPDATE `item` SET `deleted` = 1, `updated_date` = NOW()
                WHERE ({string.Join(" OR ", conditions)}) AND `deleted` = 0;
                """;
        }
    }
}
