using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for clan data management.
    /// Implements Redis value-based caching with database persistence for clan operations.
    /// </summary>
    public class ClanRepository : RedisValueRepository<Clan, ClanKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ClanRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public ClanRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves a clan by its unique identifier.
        /// </summary>
        /// <param name="id">The unique identifier of the clan.</param>
        /// <returns>The clan if found; otherwise, null.</returns>
        public async Task<Clan> Get(uint id)
        {
            return await Get(new ClanKey { Id = id });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a clan by ID.
        /// </summary>
        /// <param name="key">The clan key containing the clan ID.</param>
        /// <returns>A SQL SELECT statement for the clan.</returns>
        protected override string OnSelect(ClanKey key)
        {
            return $"""
                SELECT * FROM `clan`
                WHERE `id` = {key.Id}
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a clan with name and title information.
        /// </summary>
        /// <param name="value">The clan to upsert.</param>
        /// <returns>A SQL UPSERT statement for the clan.</returns>
        protected override string OnUpsert(Clan value)
        {
            var sql = $"""
                INSERT INTO `clan` (
                    `id`,
                    `name`,
                    `title`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Name.Escape()},
                    {value.Title.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `name`=VALUES(`name`),
                    `title`=VALUES(`title`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}