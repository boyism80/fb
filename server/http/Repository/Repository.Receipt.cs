using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ReceiptRepository : RedisHashRepository<Receipt, ReceiptKey>
    {
        public ReceiptRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<IEnumerable<Receipt>> Get(uint world, uint user)
        {
            return base.GetAll(world, new ReceiptKey
            {
                User = user,
                Id = 0
            });
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<Receipt>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<Receipt>>();

            var keys = ownerIds.Distinct().Select(uid => new ReceiptKey { User = uid, Id = 0 }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.ToDictionary(k => k.User, _ => (IList<Receipt>)new List<Receipt>());
            foreach (var row in list)
            {
                dict[row.User].Add(row);
            }

            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<Receipt>)kv.Value);
        }

        protected override string OnSelect(ReceiptKey key)
        {
            return $"""
                SELECT * FROM `receipt`
                WHERE `user` = {key.User} AND `id` = {key.Id.Escape()}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(ReceiptKey key)
        {
            return $"""
                SELECT * FROM `receipt`
                WHERE `user` = {key.User};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<ReceiptKey> keys)
        {
            return $"SELECT * FROM `receipt` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))});";
        }

        protected override ReceiptKey GetKeyFromRow(Receipt row)
        {
            return new ReceiptKey { User = row.User, Id = row.Id };
        }

        protected override string OnDelete(ReceiptKey key)
        {
            return $"""
                DELETE FROM `receipt`
                WHERE `user` = {key.User.Escape()} AND `id` = {key.Id.Escape()};
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<ReceiptKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var user = keys[0].User;
            return $"""
                DELETE FROM `receipt`
                WHERE `user` = {user.Escape()} AND `id` IN ({string.Join(",", keys.Select(k => k.Id.Escape()))});
                """;
        }

        protected override string OnUpsert(Receipt value)
        {
            return OnUpsert(new[] { value });
        }

        protected override string OnUpsert(Receipt[] values)
        {
            if (values.Length == 0)
                return string.Empty;

            var args = values.Select(value => $"""
                ({value.User.Escape()},
                 {value.Id.Escape()},
                 {value.Type.Escape()},
                 {value.Target.Escape()},
                 {value.Detail.Escape()},
                 {value.CreatedDate.Escape()},
                 {value.UpdatedDate.Escape()})
                """);

            return $"""
                INSERT INTO `receipt` (
                    `user`,
                    `id`,
                    `type`,
                    `target`,
                    `detail`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `type`=VALUES(`type`),
                    `target`=VALUES(`target`),
                    `detail`=VALUES(`detail`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}
