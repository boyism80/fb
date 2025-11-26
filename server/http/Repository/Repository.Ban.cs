using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for ban data management.
    /// Implements Redis value-based caching with database persistence for ban operations.
    /// Ban table is stored in global DB (no sharding).
    /// </summary>
    public class BanRepository : RedisValueRepository<Ban, BanKey>
    {
        private readonly DbContext _dbContext;
        private readonly RedisService _redisService;

        /// <summary>
        /// Initializes a new instance of the <see cref="BanRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public BanRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
            _dbContext = dbContext;
            _redisService = redisService;
        }

        /// <summary>
        /// Retrieves a ban by user ID.
        /// </summary>
        /// <param name="userId">The unique identifier of the user.</param>
        /// <returns>The ban if found; otherwise, null.</returns>
        public async Task<Ban> Get(uint userId)
        {
            return await Get(new BanKey { User = userId });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a ban by user ID.
        /// </summary>
        /// <param name="key">The ban key containing the user ID.</param>
        /// <returns>A SQL SELECT statement for the ban.</returns>
        protected override string OnSelect(BanKey key)
        {
            return $"""
                SELECT * FROM `ban`
                WHERE `user` = {key.User} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a ban with all ban properties.
        /// </summary>
        /// <param name="value">The ban to upsert.</param>
        /// <returns>A SQL UPSERT statement for the ban.</returns>
        protected override string OnUpsert(Ban value)
        {
            var sql = $"""
                INSERT INTO `ban` (
                    `user`,
                    `reason`,
                    `expire_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.Reason.Escape()},
                    {value.ExpireDate.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss").Escape()},
                    {value.UpdatedDate.ToString("yyyy-MM-dd HH:mm:ss").Escape()})
                ON DUPLICATE KEY UPDATE
                    `reason` = VALUES(`reason`),
                    `expire_date` = VALUES(`expire_date`),
                    `deleted` = VALUES(`deleted`),
                    `updated_date` = VALUES(`updated_date`);
                """;
            return sql;
        }

        /// <summary>
        /// Soft deletes a ban by user ID (sets deleted = 1).
        /// </summary>
        /// <param name="userId">The unique identifier of the user.</param>
        public async Task Delete(uint userId)
        {
            var ban = await Get(userId);
            if (ban == null)
                return;

            ban.Deleted = true;
            Set(ban);
        }
    }
}

