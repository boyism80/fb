using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class StorageClaimPendingRepository : RedisHashRepository<StorageClaimPending, StorageClaimPendingKey>
    {
        public StorageClaimPendingRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<IEnumerable<StorageClaimPending>> Get(uint world, uint user)
        {
            return base.GetAll(world, new StorageClaimPendingKey
            {
                User = user,
                Box = 0
            });
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<StorageClaimPending>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<StorageClaimPending>>();

            var keys = ownerIds.Distinct().Select(uid => new StorageClaimPendingKey { User = uid, Box = 0 }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.ToDictionary(k => k.User, _ => (IList<StorageClaimPending>)new List<StorageClaimPending>());
            foreach (var row in list)
            {
                dict[row.User].Add(row);
            }

            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<StorageClaimPending>)kv.Value);
        }

        protected override string OnSelect(StorageClaimPendingKey key)
        {
            return $"""
                SELECT * FROM `storage_claim_pending`
                WHERE `user` = {key.User} AND `box` = {key.Box}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(StorageClaimPendingKey key)
        {
            return $"""
                SELECT * FROM `storage_claim_pending`
                WHERE `user` = {key.User};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<StorageClaimPendingKey> keys)
        {
            return $"SELECT * FROM `storage_claim_pending` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))});";
        }

        protected override StorageClaimPendingKey GetKeyFromRow(StorageClaimPending row)
        {
            return new StorageClaimPendingKey { User = row.User, Box = row.Box };
        }

        protected override string OnDelete(StorageClaimPendingKey key)
        {
            return $"""
                DELETE FROM `storage_claim_pending`
                WHERE `user` = {key.User.Escape()} AND `box` = {key.Box.Escape()};
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<StorageClaimPendingKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var user = keys[0].User;
            return $"""
                DELETE FROM `storage_claim_pending`
                WHERE `user` = {user.Escape()} AND `box` IN ({string.Join(",", keys.Select(k => k.Box.Escape()))});
                """;
        }

        protected override string OnUpsert(StorageClaimPending value)
        {
            return OnUpsert(new[] { value });
        }

        protected override string OnUpsert(StorageClaimPending[] values)
        {
            if (values.Length == 0)
                return string.Empty;

            var args = values.Select(value => $"""
                ({value.User.Escape()},
                 {value.Box.Escape()},
                 {value.ClaimId.Escape()},
                 {value.CreatedDate.Escape()},
                 {value.UpdatedDate.Escape()})
                """);

            return $"""
                INSERT INTO `storage_claim_pending` (
                    `user`,
                    `box`,
                    `claim_id`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `claim_id`=VALUES(`claim_id`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}
