using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ItemRepository : RedisHashRepository<Item, ItemKey>
    {
        public ItemRepository(DbContext dbContext,
            RedisService redisService,
            WriteBackService dbExecuteService) : base(dbContext, redisService, dbExecuteService)
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
                    {value.Owner.Escape()},
                    {value.Index.Escape()},
                    {value.Parts.Escape()},
                    {value.Deposited.Escape()},
                    {value.Model.Escape()},
                    {value.Count.Escape()},
                    {value.Durability.Escape()},
                    {value.CustomName.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `model`=VALUES(`model`), 
                    `count`=VALUES(`count`), 
                    `durability`=VALUES(`durability`),
                    `custom_name`=VALUES(`custom_name`),
                    `deleted`=VALUES(`deleted`),
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
                         {item.Deposited.Escape()},
                         {item.Model.Escape()},
                         {item.Count.Escape()},
                         {item.Durability.Escape()},
                         {item.CustomName.Escape()},
                         {item.Deleted.Escape()},
                         {item.CreatedDate.Escape()},
                         {item.UpdatedDate.Escape()})
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
                        `model`=VALUES(`model`),
                        `count`=VALUES(`count`),
                        `durability`=VALUES(`durability`),
                        `custom_name`=VALUES(`custom_name`),
                        `deleted`=VALUES(`deleted`);
                    """;

            return sql;
        }
    }
}