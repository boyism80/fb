using Db.Extension;
using Db.Model;
using Db.Service;
using http.Service;

namespace Db.Reepository
{
    public class ItemRepository : RedisHashRepository<Item, ItemKey>
    {
        public ItemRepository(DbContext dbContext,
            RedisService redisService,
            DbExecuteService dbExecuteService) : base(dbContext, redisService, dbExecuteService)
        {
        }

        public async Task<Item> Get(uint owner, short index, short parts, short deposited)
        {
            return await Get(new ItemKey
            {
                Owner = owner,
                Index = index,
                Parts = parts,
                Deposited = deposited
            });
        }

        public async Task<IEnumerable<Item>> Get(uint owner)
        {
            return await GetAll(new ItemKey
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
                `deposited` = {key.Deposited}
                LIMIT 1;
                """";
        }

        protected override string OnSelectBulk(ItemKey key)
        {
            return $"""
                SELECT * FROM `item` WHERE
                `owner` = {key.Owner};
                """;
        }

        protected override string OnUpsert(Item value)
        {
            var customName = value.CustomName;
            if (string.IsNullOrEmpty(customName))
                customName = "NULL";
            else
                customName = $"\"{customName}\"";

            var sql = $"""
                INSERT INTO item (
                    `owner`,
                    `index`,
                    `parts`,
                    `deposited`,
                    `model`,
                    `count`,
                    `durability`,
                    `custom_name`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Owner},
                    {value.Index},
                    {value.Parts},
                    {value.Deposited},
                    {value.Model},
                    {value.Count},
                    {value.Durability.Escape()},
                    {customName},
                    {value.Deleted},
                    '{value.CreatedDate:yyyy-MM-dd HH:mm:ss.ffffff}',
                    '{value.UpdatedDate:yyyy-MM-dd HH:mm:ss.ffffff}')
                ON DUPLICATE KEY UPDATE 
                    model=VALUES(model), 
                    count=VALUES(count), 
                    durability=VALUES(durability),
                    custom_name=VALUES(custom_name),
                    deleted=VALUES(deleted),
                    updated_date=VALUES(updated_date);";
                """;

            return sql;
        }

        protected override string OnUpsert(Item[] values)
        {
            var args = values.Select(item =>
            {
                var customName = item.CustomName;
                if (string.IsNullOrEmpty(customName))
                    customName = "NULL";
                else
                    customName = $"\"{customName}\"";

                return $"""
                        ({item.Owner},
                         {item.Index},
                         {item.Parts},
                         {item.Deposited},
                         {item.Model},
                         {item.Count},
                         {item.Durability.Escape()},
                         {customName},
                         {item.Deleted},
                         '{item.CreatedDate:yyyy-MM-dd HH:mm:ss.ffffff}',
                         '{item.UpdatedDate:yyyy-MM-dd HH:mm:ss.ffffff}')
                        """;
            });

            var sql = $"""
                    INSERT INTO item (
                        `owner`,
                        `index`,
                        `parts`,
                        `deposited`,
                        `model`,
                        `count`,
                        `durability`,
                        `custom_name`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        model=VALUES(model),
                        count=VALUES(count),
                        durability=VALUES(durability),
                        custom_name=VALUES(custom_name),
                        deleted=VALUES(deleted);";
                    """;

            return sql;
        }
    }
}