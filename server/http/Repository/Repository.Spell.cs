using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for spell data management.
    /// Implements Redis hash-based caching with database persistence for spell operations.
    /// </summary>
    public class SpellRepository : RedisHashRepository<Spell, SpellKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SpellRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public SpellRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {

        }

        /// <summary>
        /// Retrieves a specific spell for a character by owner ID and slot number.
        /// </summary>
        /// <param name="owner">The unique identifier of the character who owns the spell.</param>
        /// <param name="slot">The spell slot number where the spell is stored.</param>
        /// <returns>The spell if found; otherwise, null.</returns>
        public async Task<Spell> Get(uint owner, byte slot)
        {
            return await base.Get(new SpellKey
            {
                Owner = owner,
                Slot = slot
            });
        }

        /// <summary>
        /// Retrieves all spells for a specific character by owner ID.
        /// </summary>
        /// <param name="owner">The unique identifier of the character who owns the spells.</param>
        /// <returns>A collection of all spells for the specified character.</returns>
        public async Task<IEnumerable<Spell>> Get(uint owner)
        {
            return await base.GetAll(new SpellKey
            {
                Owner = owner
            });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a specific spell.
        /// </summary>
        /// <param name="key">The spell key containing owner ID and slot number.</param>
        /// <returns>A SQL SELECT statement for the specific spell.</returns>
        protected override string OnSelect(SpellKey key)
        {
            var sql = $"""
                SELECT * FROM `spell` WHERE 
                `owner` = {key.Owner} AND
                `slot` = {key.Slot}
                LIMIT 1;
                """;

            return sql;
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving all spells for a character.
        /// </summary>
        /// <param name="key">The spell key containing the owner ID.</param>
        /// <returns>A SQL SELECT statement for all spells of the specified character.</returns>
        protected override string OnSelectBulk(SpellKey key)
        {
            var sql = $"""
                SELECT * FROM `spell` WHERE
                `owner` = {key.Owner};
                """;
            return sql;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a single spell.
        /// </summary>
        /// <param name="value">The spell to upsert.</param>
        /// <returns>A SQL UPSERT statement for the spell.</returns>
        protected override string OnUpsert(Spell value)
        {
            var sql = $"""
                    INSERT INTO spell (
                        `owner`,
                        `slot`,
                        `model`,
                        `next`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES (
                        {value.Owner.Escape()},
                        {value.Slot.Escape()},
                        {value.Model.Escape()},
                        {value.Next.Escape()},
                        {value.Deleted.Escape()},
                        {value.CreatedDate.Escape()},
                        {value.UpdatedDate.Escape()})
                    ON DUPLICATE KEY UPDATE
                        `model`=VALUES(`model`),
                        `next`=VALUES(`next`),
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for multiple spells in a batch operation.
        /// </summary>
        /// <param name="values">The array of spells to upsert.</param>
        /// <returns>A SQL UPSERT statement for the batch of spells.</returns>
        protected override string OnUpsert(Spell[] values)
        {
            var args = values.Select(spell =>
            {
                return $"""
                        ({spell.Owner.Escape()},
                         {spell.Slot.Escape()},
                         {spell.Model.Escape()},
                         {spell.Next.Escape()},
                         {spell.Deleted.Escape()},
                         {spell.CreatedDate.Escape()},
                         {spell.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO spell (
                        `owner`,
                        `slot`,
                        `model`,
                        `next`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `model`=VALUES(`model`),
                        `next`=VALUES(`next`),
                        `deleted`=VALUES(`deleted`),
                        `created_date`=VALUES(`created_date`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}