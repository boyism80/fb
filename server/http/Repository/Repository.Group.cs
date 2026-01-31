using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for group data management.
    /// Implements Redis value-based caching with database persistence for group operations.
    /// </summary>
    public class GroupRepository : RedisValueRepository<Group, GroupKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="GroupRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public GroupRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves a group by world and the master (leader) character ID.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="master">The unique identifier of the group master/leader.</param>
        /// <returns>The group if found; otherwise, null.</returns>
        public async Task<Group> Get(uint world, uint master)
        {
            return await base.Get(world, new GroupKey { Master = master });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a group by master ID.
        /// </summary>
        /// <param name="key">The group key containing the master ID.</param>
        /// <returns>A SQL SELECT statement for the group.</returns>
        protected override string OnSelect(GroupKey key)
        {
            return $"""
                SELECT * FROM `group`
                WHERE `master` = {key.Master}
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a group with member list serialization.
        /// Serializes the member list as JSON for database storage.
        /// </summary>
        /// <param name="value">The group to upsert.</param>
        /// <returns>A SQL UPSERT statement for the group.</returns>
        protected override string OnUpsert(Group value)
        {
            var sql = $"""
                INSERT INTO `group` (
                    `master`,
                    `members`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Master.Escape()},
                    {JsonConvert.SerializeObject(value.Members).Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `members`=VALUES(`members`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}