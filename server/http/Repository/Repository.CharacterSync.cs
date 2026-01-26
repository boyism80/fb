using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for character synchronization data management.
    /// Implements Redis value-based caching with database persistence for character sync operations.
    /// </summary>
    public class CharacterSyncRepository : RedisValueRepository<CharacterSync, CharacterSyncKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CharacterSyncRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public CharacterSyncRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves character synchronization data by section and character unique identifier.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="uid">The unique identifier of the character.</param>
        /// <returns>The character synchronization data if found; otherwise, null.</returns>
        public async Task<CharacterSync> Get(string section, uint uid)
        {
            return await base.Get(section, new CharacterSyncKey { Uid = uid });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving character sync data by character ID.
        /// </summary>
        /// <param name="key">The character sync key containing the character ID.</param>
        /// <returns>A SQL SELECT statement for the character synchronization data.</returns>
        protected override string OnSelect(CharacterSyncKey key)
        {
            return $"""
                SELECT * FROM `character_sync`
                WHERE `uid` = {key.Uid}
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for character synchronization data.
        /// Includes group and clan association information for cross-server synchronization.
        /// </summary>
        /// <param name="value">The character synchronization data to upsert.</param>
        /// <returns>A SQL UPSERT statement for the character sync data.</returns>
        protected override string OnUpsert(CharacterSync value)
        {
            var sql = $"""
                INSERT INTO `character_sync` (
                    `uid`,
                    `group`,
                    `clan`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Uid.Escape()},
                    {value.Group.Escape()},
                    {value.Clan.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `group`=VALUES(`group`),
                    `clan`=VALUES(`clan`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}