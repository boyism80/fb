using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class SpellRepository : RedisHashRepository<Spell, SpellKey>
    {
        public SpellRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {

        }

        public async Task<Spell> Get(uint world, uint owner, byte slot)
        {
            return await base.Get(world, new SpellKey
            {
                Owner = owner,
                Slot = slot
            });
        }

        public async Task<IEnumerable<Spell>> Get(uint world, uint owner)
        {
            return await base.GetAll(world, new SpellKey
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

        protected override string OnSelectMany(IReadOnlyList<SpellKey> keys)
        {
            return $"SELECT * FROM `spell` WHERE `owner` IN ({string.Join(",", keys.Select(k => k.Owner))});";
        }

        protected override SpellKey GetKeyFromRow(Spell row)
        {
            return new SpellKey { Owner = row.Owner };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<Spell>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<Spell>>();

            var keys = ownerIds.Distinct().Select(oid => new SpellKey { Owner = oid }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.Owner, _ => (IList<Spell>)new List<Spell>());
            foreach (var s in list)
            {
                dict[s.Owner].Add(s);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<Spell>)kv.Value);
        }

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
