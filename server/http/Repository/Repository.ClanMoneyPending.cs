using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ClanMoneyPendingRepository : RedisHashRepository<ClanMoneyPending, ClanMoneyPendingKey>
    {
        public ClanMoneyPendingRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<IEnumerable<ClanMoneyPending>> Get(uint world, uint user)
        {
            return base.GetAll(world, new ClanMoneyPendingKey
            {
                User = user,
                RequestId = 0
            });
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<ClanMoneyPending>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<ClanMoneyPending>>();

            var keys = ownerIds.Distinct().Select(uid => new ClanMoneyPendingKey { User = uid, RequestId = 0 }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.ToDictionary(k => k.User, _ => (IList<ClanMoneyPending>)new List<ClanMoneyPending>());
            foreach (var row in list)
            {
                dict[row.User].Add(row);
            }

            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<ClanMoneyPending>)kv.Value);
        }

        protected override string OnSelect(ClanMoneyPendingKey key)
        {
            return $"""
                SELECT * FROM `clan_money_pending`
                WHERE `user` = {key.User} AND `request_id` = {key.RequestId.Escape()}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(ClanMoneyPendingKey key)
        {
            return $"""
                SELECT * FROM `clan_money_pending`
                WHERE `user` = {key.User};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<ClanMoneyPendingKey> keys)
        {
            return $"SELECT * FROM `clan_money_pending` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))});";
        }

        protected override ClanMoneyPendingKey GetKeyFromRow(ClanMoneyPending row)
        {
            return new ClanMoneyPendingKey { User = row.User, RequestId = row.RequestId };
        }

        protected override string OnDelete(ClanMoneyPendingKey key)
        {
            return $"""
                DELETE FROM `clan_money_pending`
                WHERE `user` = {key.User.Escape()} AND `request_id` = {key.RequestId.Escape()};
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<ClanMoneyPendingKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var user = keys[0].User;
            return $"""
                DELETE FROM `clan_money_pending`
                WHERE `user` = {user.Escape()} AND `request_id` IN ({string.Join(",", keys.Select(k => k.RequestId.Escape()))});
                """;
        }

        protected override string OnUpsert(ClanMoneyPending value)
        {
            return OnUpsert(new[] { value });
        }

        protected override string OnUpsert(ClanMoneyPending[] values)
        {
            if (values.Length == 0)
                return string.Empty;

            var args = values.Select(value => $"""
                ({value.User.Escape()},
                 {value.RequestId.Escape()},
                 {value.Clan.Escape()},
                 {value.ClanDelta.Escape()},
                 {value.MoneyDelta.Escape()},
                 {value.CreatedDate.Escape()},
                 {value.UpdatedDate.Escape()})
                """);

            return $"""
                INSERT INTO `clan_money_pending` (
                    `user`,
                    `request_id`,
                    `clan`,
                    `clan_delta`,
                    `money_delta`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `clan`=VALUES(`clan`),
                    `clan_delta`=VALUES(`clan_delta`),
                    `money_delta`=VALUES(`money_delta`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}
