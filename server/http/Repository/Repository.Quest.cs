using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for item data management.
    /// Implements Redis hash-based caching with database persistence for item operations.
    /// </summary>
    public class QuestRepository : RedisHashRepository<Quest, QuestKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="QuestRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public QuestRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves a specific item by its complete identification parameters.
        /// </summary>
        /// <param name="owner">The unique identifier of the character who owns the item.</param>
        /// <param name="index">The inventory slot index where the item is located.</param>
        /// <param name="parts">The equipment parts identifier for the item.</param>
        /// <param name="stored">The storage type identifier where the item is stored.</param>
        /// <returns>The item if found; otherwise, null.</returns>
        public async Task<Quest> Get(uint user, uint id)
        {
            return await Get(new QuestKey
            {
                User = user,
                Id = id
            });
        }

        /// <summary>
        /// Retrieves all items belonging to a specific character.
        /// </summary>
        /// <param name="user">The unique identifier of the character who owns the items.</param>
        /// <returns>A collection of all items belonging to the specified character.</returns>
        public async Task<IEnumerable<Quest>> Get(uint user)
        {
            return await GetAll(new QuestKey
            {
                User = user
            });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a specific item.
        /// </summary>
        /// <param name="key">The item key containing user, id information.</param>
        /// <returns>A SQL SELECT statement for the specific item.</returns>
        protected override string OnSelect(QuestKey key)
        {
            return $""""
                SELECT * FROM `quest` WHERE 
                `user` = {key.User} AND
                `id` = {key.Id}
                LIMIT 1;
                """";
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving all items for a character.
        /// </summary>
        /// <param name="key">The item key containing the owner identifier.</param>
        /// <returns>A SQL SELECT statement for all items of the specified character.</returns>
        protected override string OnSelectBulk(QuestKey key)
        {
            return $"""
                SELECT * FROM `quest` WHERE
                `user` = {key.User};
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a single item.
        /// </summary>
        /// <param name="value">The item to upsert.</param>
        /// <returns>A SQL UPSERT statement for the item.</returns>
        protected override string OnUpsert(Quest value)
        {
            var sql = $"""
                INSERT INTO `quest` (
                    `user`,
                    `id`,
                    `step`,
                    `progress`,
                    `completed`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.Id.Escape()},
                    {value.Step.Escape()},
                    {value.Progress.Escape()},
                    {value.Completed.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `step`=VALUES(`step`), 
                    `progress`=VALUES(`progress`), 
                    `completed`=VALUES(`completed`),
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
        protected override string OnUpsert(Quest[] values)
        {
            var args = values.Select(item =>
            {
                return $"""
                        ({item.User.Escape()},
                         {item.Id.Escape()},
                         {item.Step.Escape()},
                         {item.Progress.Escape()},
                         {item.Completed.Escape()},
                         {item.Deleted.Escape()},
                         {item.CreatedDate.Escape()},
                         {item.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO quest (
                        `user`,
                        `id`,
                        `step`,
                        `progress`,
                        `completed`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `step`=VALUES(`step`),
                        `progress`=VALUES(`progress`),
                        `completed`=VALUES(`completed`),
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}