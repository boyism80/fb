using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for user option data management.
    /// Implements Redis value-based caching with database persistence for option operations.
    /// </summary>
    public class OptionRepository : RedisValueRepository<Option, OptionKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="OptionRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public OptionRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }


        /// <summary>
        /// Retrieves user options by section and their unique identifier.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="uid">The unique identifier of the user whose options to retrieve.</param>
        /// <returns>The user options if found; otherwise, null.</returns>
        public async Task<Option> Get(string section, uint uid)
        {
            return await base.Get(section, new OptionKey { Uid = uid });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving user options by ID.
        /// </summary>
        /// <param name="key">The option key containing the user ID.</param>
        /// <returns>A SQL SELECT statement for the user options.</returns>
        protected override string OnSelect(OptionKey key)
        {
            return $"""
                SELECT * FROM `option`
                WHERE `uid` = {key.Uid}
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for user options with all preference settings.
        /// Includes all boolean flags for game features and user interface preferences.
        /// </summary>
        /// <param name="value">The user options to upsert.</param>
        /// <returns>A SQL UPSERT statement for the user options.</returns>
        protected override string OnUpsert(Option value)
        {
            var sql = $"""
                INSERT INTO `option` (
                    `uid`,
                    `whisper`,
                    `group`,
                    `roar`,
                    `roar_worlds`,
                    `magic_effect`,
                    `weather_effect`,
                    `fixed_move`,
                    `trade`,
                    `fast_move`,
                    `effect_sound`,
                    `pk_protect`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Uid.Escape()},
                    {value.Whisper.Escape()},
                    {value.Group.Escape()},
                    {value.Roar.Escape()},
                    {value.RoarWorlds.Escape()},
                    {value.MagicEffect.Escape()},
                    {value.WeatherEffect.Escape()},
                    {value.FixedMove.Escape()},
                    {value.Trade.Escape()},
                    {value.FastMove.Escape()},
                    {value.EffectSound.Escape()},
                    {value.PkProtect.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `whisper`=VALUES(`whisper`),
                    `group`=VALUES(`group`),
                    `roar`=VALUES(`roar`),
                    `roar_worlds`=VALUES(`roar_worlds`),
                    `magic_effect`=VALUES(`magic_effect`),
                    `weather_effect`=VALUES(`weather_effect`),
                    `fixed_move`=VALUES(`fixed_move`),
                    `trade`=VALUES(`trade`),
                    `fast_move`=VALUES(`fast_move`),
                    `effect_sound`=VALUES(`effect_sound`),
                    `pk_protect`=VALUES(`pk_protect`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}