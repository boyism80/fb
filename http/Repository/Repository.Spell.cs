using Http.Extension;
using Http.Model;
using Http.Service;
using Http.Service;

namespace Http.Reepository
{
    public class SpellRepository : RedisHashRepository<Spell, SpellKey>
    {
        public SpellRepository(DbContext dbContext,
            RedisService redisService,
            WriteBackService dbExecuteService) : base(dbContext, redisService, dbExecuteService)
        {

        }

        public async Task<Spell> Get(uint owner, byte slot)
        {
            return await base.Get(new SpellKey
            {
                Owner = owner,
                Slot = slot
            });
        }

        public async Task<IEnumerable<Spell>> Get(uint owner)
        {
            return await base.GetAll(new SpellKey
            {
                Owner = owner
            });
        }

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

        protected override string OnSelectBulk(SpellKey key)
        {
            var sql = $"""
                SELECT * FROM `spell` WHERE
                `owner` = {key.Owner};
                """;
            return sql;
        }

        protected override string OnUpsert(Spell value)
        {
            var sql = $"""
                    INSERT INTO spell (
                        `owner`,
                        `slot`,
                        `model`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES (
                        {value.Owner.Escape()},
                        {value.Slot.Escape()},
                        {value.Model.Escape()},
                        {value.Deleted.Escape()},
                        {value.CreatedDate.Escape()},
                        {value.UpdatedDate.Escape()})
                    ON DUPLICATE KEY UPDATE
                        `model`=VALUES(`model`),
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }

        protected override string OnUpsert(Spell[] values)
        {
            var args = values.Select(spell =>
            {
                return $"""
                        ({spell.Owner.Escape()},
                         {spell.Slot.Escape()},
                         {spell.Model.Escape()},
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
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `model`=VALUES(`model`),
                        `deleted`=VALUES(`deleted`),
                        `created_date`=VALUES(`created_date`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}