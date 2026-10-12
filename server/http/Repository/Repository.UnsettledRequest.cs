using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class UnsettledRequestRepository : RedisHashRepository<UnsettledRequest, UnsettledRequestKey>
    {
        public UnsettledRequestRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<IEnumerable<UnsettledRequest>> Get(uint world, uint user)
        {
            return base.GetAll(world, new UnsettledRequestKey
            {
                User = user,
                Id = 0
            });
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<UnsettledRequest>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<UnsettledRequest>>();

            var keys = ownerIds.Distinct().Select(uid => new UnsettledRequestKey { User = uid, Id = 0 }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.ToDictionary(k => k.User, _ => (IList<UnsettledRequest>)new List<UnsettledRequest>());
            foreach (var row in list)
            {
                dict[row.User].Add(row);
            }

            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<UnsettledRequest>)kv.Value);
        }

        protected override string OnSelect(UnsettledRequestKey key)
        {
            return $"""
                SELECT * FROM `unsettled_request`
                WHERE `user` = {key.User} AND `id` = {key.Id.Escape()}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(UnsettledRequestKey key)
        {
            return $"""
                SELECT * FROM `unsettled_request`
                WHERE `user` = {key.User};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<UnsettledRequestKey> keys)
        {
            return $"SELECT * FROM `unsettled_request` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))});";
        }

        protected override UnsettledRequestKey GetKeyFromRow(UnsettledRequest row)
        {
            return new UnsettledRequestKey { User = row.User, Id = row.Id };
        }

        protected override string OnDelete(UnsettledRequestKey key)
        {
            return $"""
                DELETE FROM `unsettled_request`
                WHERE `user` = {key.User.Escape()} AND `id` = {key.Id.Escape()};
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<UnsettledRequestKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var user = keys[0].User;
            return $"""
                DELETE FROM `unsettled_request`
                WHERE `user` = {user.Escape()} AND `id` IN ({string.Join(",", keys.Select(k => k.Id.Escape()))});
                """;
        }

        protected override string OnUpsert(UnsettledRequest value)
        {
            return OnUpsert(new[] { value });
        }

        protected override string OnUpsert(UnsettledRequest[] values)
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
                INSERT INTO `unsettled_request` (
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
