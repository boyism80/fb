using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for achievement data management.
    /// Implements Redis hash-based caching with database persistence for achievement operations.
    /// </summary>
    public class AchievementRepository : RedisHashRepository<Achievement, AchievementKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="AchievementRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public AchievementRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {

        }

        /// <summary>
        /// Retrieves a specific achievement for a character by user ID and achievement model.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="uid">The unique identifier of the character.</param>
        /// <param name="model">The achievement model identifier.</param>
        /// <returns>The achievement if found; otherwise, null.</returns>
        public async Task<Achievement> Get(string section, uint uid, uint model)
        {
            return await base.Get(section, new AchievementKey
            {
                Uid = uid,
                Model = model
            });
        }

        /// <summary>
        /// Retrieves all achievements for a specific character by user ID.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="uid">The unique identifier of the character.</param>
        /// <returns>A collection of all achievements for the specified character.</returns>
        public async Task<IEnumerable<Achievement>> Get(string section, uint uid)
        {
            return await base.GetAll(section, new AchievementKey
            {
                Uid = uid
            });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a specific achievement.
        /// </summary>
        /// <param name="key">The achievement key containing user ID and model identifier.</param>
        /// <returns>A SQL SELECT statement for the specific achievement.</returns>
        protected override string OnSelect(AchievementKey key)
        {
            var sql = $"""
                SELECT * FROM `achievement` WHERE 
                `uid` = {key.Uid} AND
                `model` = {key.Model}
                LIMIT 1;
                """;

            return sql;
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving all achievements for a character.
        /// </summary>
        /// <param name="key">The achievement key containing the user ID.</param>
        /// <returns>A SQL SELECT statement for all achievements of the specified character.</returns>
        protected override string OnSelectBulk(AchievementKey key)
        {
            var sql = $"""
                SELECT * FROM `achievement` WHERE
                `uid` = {key.Uid};
                """;
            return sql;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a single achievement.
        /// </summary>
        /// <param name="value">The achievement to upsert.</param>
        /// <returns>A SQL UPSERT statement for the achievement.</returns>
        protected override string OnUpsert(Achievement value)
        {
            var sql = $"""
                    INSERT INTO `achievement` (
                        `uid`,
                        `model`,
                        `text`,
                        `icon`,
                        `color`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES (
                        {value.Uid.Escape()},
                        {value.Model.Escape()},
                        {value.Text.Escape()},
                        {value.Icon.Escape()},
                        {value.Color.Escape()},
                        {value.Deleted.Escape()},
                        {value.CreatedDate.Escape()},
                        {value.UpdatedDate.Escape()})
                    ON DUPLICATE KEY UPDATE
                        `text`=VALUES(`text`),
                        `icon`=VALUES(`icon`),
                        `color`=VALUES(`color`),
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for multiple achievements in a batch operation.
        /// </summary>
        /// <param name="values">The array of achievements to upsert.</param>
        /// <returns>A SQL UPSERT statement for the batch of achievements.</returns>
        protected override string OnUpsert(Achievement[] values)
        {
            var args = values.Select(achievement =>
            {
                return $"""
                        ({achievement.Uid.Escape()},
                         {achievement.Model.Escape()},
                         {achievement.Text.Escape()},
                         {achievement.Icon.Escape()},
                         {achievement.Color.Escape()},
                         {achievement.Deleted.Escape()},
                         {achievement.CreatedDate.Escape()},
                         {achievement.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO `achievement` (
                        `uid`,
                        `model`,
                        `text`,
                        `icon`,
                        `color`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `text`=VALUES(`text`),
                        `icon`=VALUES(`icon`),
                        `color`=VALUES(`color`),
                        `deleted`=VALUES(`deleted`),
                        `created_date`=VALUES(`created_date`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}