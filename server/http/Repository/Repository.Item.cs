using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for item data management.
    /// Implements Redis hash-based caching with database persistence for item operations.
    /// </summary>
    public class ItemRepository : RedisHashRepository<Item, ItemKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ItemRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public ItemRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves a specific item by its complete identification parameters.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="owner">The unique identifier of the character who owns the item.</param>
        /// <param name="index">The inventory slot index where the item is located.</param>
        /// <param name="parts">The equipment parts identifier for the item.</param>
        /// <param name="stored">The storage type identifier where the item is stored.</param>
        /// <returns>The item if found; otherwise, null.</returns>
        public async Task<Item> Get(string section, uint owner, short index, short parts, short stored)
        {
            return await base.Get(section, new ItemKey
            {
                Owner = owner,
                Index = index,
                Parts = parts,
                Stored = stored
            });
        }

        /// <summary>
        /// Retrieves all items belonging to a specific character.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="owner">The unique identifier of the character who owns the items.</param>
        /// <returns>A collection of all items belonging to the specified character.</returns>
        public async Task<IEnumerable<Item>> Get(string section, uint owner)
        {
            return await base.GetAll(section, new ItemKey
            {
                Owner = owner
            });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a specific item.
        /// </summary>
        /// <param name="key">The item key containing owner, index, parts, and storage information.</param>
        /// <returns>A SQL SELECT statement for the specific item.</returns>
        protected override string OnSelect(ItemKey key)
        {
            return $""""
                SELECT * FROM `item` WHERE 
                `owner` = {key.Owner} AND
                `index` = {key.Index} AND
                `parts` = {key.Parts} AND
                `stored` = {key.Stored}
                LIMIT 1;
                """";
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving all items for a character.
        /// </summary>
        /// <param name="key">The item key containing the owner identifier.</param>
        /// <returns>A SQL SELECT statement for all items of the specified character.</returns>
        protected override string OnSelectBulk(ItemKey key)
        {
            return $"""
                SELECT * FROM `item` WHERE
                `owner` = {key.Owner};
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a single item.
        /// </summary>
        /// <param name="value">The item to upsert.</param>
        /// <returns>A SQL UPSERT statement for the item.</returns>
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

        /// <summary>
        /// Generates the SQL UPSERT statement for multiple items in a batch operation.
        /// </summary>
        /// <param name="values">The array of items to upsert.</param>
        /// <returns>A SQL UPSERT statement for the batch of items.</returns>
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
                        `stored`,
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
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}