using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class StorageRewardMarkRepository : RedisHashRepository<StorageRewardMark, StorageRewardMarkKey>
    {
        public StorageRewardMarkRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<IEnumerable<StorageRewardMark>> Get(uint world, uint user)
        {
            return base.GetAll(world, new StorageRewardMarkKey
            {
                User = user
            });
        }

        public Task<StorageRewardMark> Get(uint world, uint user, string pendingId)
        {
            return base.Get(world, new StorageRewardMarkKey
            {
                User = user,
                PendingId = pendingId
            });
        }

        protected override string OnSelect(StorageRewardMarkKey key)
        {
            return $"""
                SELECT * FROM `storage_reward_mark`
                WHERE `pending_id` = {key.PendingId.Escape()}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(StorageRewardMarkKey key)
        {
            return $"""
                SELECT * FROM `storage_reward_mark`
                WHERE `user` = {key.User};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<StorageRewardMarkKey> keys)
        {
            return $"SELECT * FROM `storage_reward_mark` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))});";
        }

        protected override StorageRewardMarkKey GetKeyFromRow(StorageRewardMark row)
        {
            return new StorageRewardMarkKey { User = row.User };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<StorageRewardMark>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<StorageRewardMark>>();

            var keys = ownerIds.Distinct().Select(uid => new StorageRewardMarkKey { User = uid }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.User, _ => (IList<StorageRewardMark>)new List<StorageRewardMark>());
            foreach (var r in list)
            {
                dict[r.User].Add(r);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<StorageRewardMark>)kv.Value);
        }

        protected override string OnUpsert(StorageRewardMark value)
        {
            return $"""
                INSERT INTO `storage_reward_mark` (
                    `user`,
                    `pending_id`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.PendingId.Escape()},
                    {value.ExpiredDate.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `expired_date`=VALUES(`expired_date`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }

        protected override string OnUpsert(StorageRewardMark[] values)
        {
            var args = values.Select(value =>
            {
                return $"""
                        ({value.User.Escape()},
                         {value.PendingId.Escape()},
                         {value.ExpiredDate.Escape()},
                         {value.Deleted.Escape()},
                         {value.CreatedDate.Escape()},
                         {value.UpdatedDate.Escape()})
                        """;
            });

            return $"""
                INSERT INTO `storage_reward_mark` (
                    `user`,
                    `pending_id`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `expired_date`=VALUES(`expired_date`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}
