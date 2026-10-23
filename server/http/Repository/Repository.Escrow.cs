using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class EscrowRepository : RedisHashRepository<Escrow, EscrowKey>
    {
        public EscrowRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<IEnumerable<Escrow>> Get(uint world, uint user)
        {
            return base.GetAll(world, new EscrowKey
            {
                User = user,
                Id = 0
            });
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<Escrow>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
            {
                return new Dictionary<uint, IReadOnlyList<Escrow>>();
            }

            var keys = ownerIds.Distinct().Select(uid => new EscrowKey { User = uid, Id = 0 }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.ToDictionary(k => k.User, _ => (IList<Escrow>)new List<Escrow>());
            foreach (var row in list)
            {
                dict[row.User].Add(row);
            }

            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<Escrow>)kv.Value);
        }

        protected override string OnSelect(EscrowKey key)
        {
            return $"""
                SELECT * FROM `escrow`
                WHERE `user` = {key.User} AND `id` = {key.Id.Escape()}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(EscrowKey key)
        {
            return $"""
                SELECT * FROM `escrow`
                WHERE `user` = {key.User};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<EscrowKey> keys)
        {
            return $"SELECT * FROM `escrow` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))});";
        }

        protected override EscrowKey GetKeyFromRow(Escrow row)
        {
            return new EscrowKey { User = row.User, Id = row.Id };
        }

        protected override string OnDelete(EscrowKey key)
        {
            return $"""
                DELETE FROM `escrow`
                WHERE `user` = {key.User.Escape()} AND `id` = {key.Id.Escape()};
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<EscrowKey> keys)
        {
            if (keys.Count == 0)
            {
                return string.Empty;
            }

            var user = keys[0].User;
            return $"""
                DELETE FROM `escrow`
                WHERE `user` = {user.Escape()} AND `id` IN ({string.Join(",", keys.Select(k => k.Id.Escape()))});
                """;
        }

        protected override string OnUpsert(Escrow value)
        {
            return OnUpsert(new[] { value });
        }

        protected override string OnUpsert(Escrow[] values)
        {
            if (values.Length == 0)
            {
                return string.Empty;
            }

            var args = values.Select(value => $"""
                ({value.User.Escape()},
                 {value.Id.Escape()},
                 {value.Money.Escape()},
                 {value.CreatedDate.Escape()},
                 {value.UpdatedDate.Escape()})
                """);

            return $"""
                INSERT INTO `escrow` (
                    `user`,
                    `id`,
                    `money`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `money`=VALUES(`money`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}
