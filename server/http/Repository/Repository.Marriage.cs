using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for marriage data management.
    /// Implements Redis value-based caching with database persistence for marriage state (spouse_id, remarriage_after, divorce_count).
    /// </summary>
    public class MarriageRepository : RedisValueRepository<Marriage, MarriageKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="MarriageRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public MarriageRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves marriage state by world and character id.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="characterId">The character id.</param>
        /// <returns>The marriage row if found; otherwise, null (character never married).</returns>
        public async Task<Marriage> Get(uint world, uint characterId)
        {
            return await base.Get(world, new MarriageKey { CharacterId = characterId });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving marriage by character id.
        /// </summary>
        /// <param name="key">The marriage key containing the character id.</param>
        /// <returns>A SQL SELECT statement for the marriage row.</returns>
        protected override string OnSelect(MarriageKey key)
        {
            return $"""
                SELECT
                    character_id AS CharacterId,
                    spouse_id AS SpouseId,
                    COALESCE(remarriage_after, NOW()) AS RemarriageAfter,
                    divorce_count AS DivorceCount,
                    created_date AS CreatedDate,
                    updated_date AS UpdatedDate
                FROM `marriage`
                WHERE `character_id` = {key.CharacterId}
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for marriage state.
        /// </summary>
        /// <param name="value">The marriage entity to upsert.</param>
        /// <returns>A SQL UPSERT statement for the marriage row.</returns>
        protected override string OnUpsert(Marriage value)
        {
            var spouseIdSql = value.SpouseId.HasValue ? value.SpouseId.Value.Escape() : "NULL";
            var remarriageAfterSql = value.RemarriageAfter.ToString("yyyy-MM-dd HH:mm:ss").Escape();

            var sql = $"""
                INSERT INTO `marriage` (
                    `character_id`,
                    `spouse_id`,
                    `remarriage_after`,
                    `divorce_count`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.CharacterId.Escape()},
                    {spouseIdSql},
                    {remarriageAfterSql},
                    {value.DivorceCount.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `spouse_id` = VALUES(`spouse_id`),
                    `remarriage_after` = VALUES(`remarriage_after`),
                    `divorce_count` = VALUES(`divorce_count`),
                    `updated_date` = VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}
