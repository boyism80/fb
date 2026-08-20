using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class CollectionUnlockRepository : RedisHashRepository<CollectionUnlock, CollectionUnlockKey>
    {
        public CollectionUnlockRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<CollectionUnlock> Get(uint world, uint user, uint mobId)
        {
            return await base.Get(world, new CollectionUnlockKey
            {
                User = user,
                MobId = mobId
            });
        }

        public async Task<IEnumerable<CollectionUnlock>> Get(uint world, uint user)
        {
            return await base.GetAll(world, new CollectionUnlockKey
            {
                User = user
            });
        }

        protected override string OnSelect(CollectionUnlockKey key)
        {
            return $"""
                SELECT * FROM `collection_unlock` WHERE
                `user` = {key.User} AND
                `mob_id` = {key.MobId} AND
                `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(CollectionUnlockKey key)
        {
            return $"""
                SELECT * FROM `collection_unlock` WHERE
                `user` = {key.User} AND `deleted` = 0;
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<CollectionUnlockKey> keys)
        {
            return $"SELECT * FROM `collection_unlock` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))}) AND `deleted` = 0;";
        }

        protected override CollectionUnlockKey GetKeyFromRow(CollectionUnlock row)
        {
            return new CollectionUnlockKey { User = row.User };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<CollectionUnlock>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<CollectionUnlock>>();

            var keys = ownerIds.Distinct().Select(uid => new CollectionUnlockKey { User = uid }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.User, _ => (IList<CollectionUnlock>)new List<CollectionUnlock>());
            foreach (var row in list)
            {
                dict[row.User].Add(row);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<CollectionUnlock>)kv.Value);
        }

        protected override string OnUpsert(CollectionUnlock value)
        {
            return $"""
                INSERT INTO `collection_unlock` (
                    `user`,
                    `mob_id`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.MobId.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `deleted`=0,
                    `updated_date`=VALUES(`updated_date`);
                """;
        }

        protected override string OnUpsert(CollectionUnlock[] values)
        {
            var args = values.Select(item =>
            {
                return $"""
                        ({item.User.Escape()},
                         {item.MobId.Escape()},
                         0,
                         {item.CreatedDate.Escape()},
                         {item.UpdatedDate.Escape()})
                        """;
            });

            return $"""
                    INSERT INTO `collection_unlock` (
                        `user`,
                        `mob_id`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `deleted`=0,
                        `updated_date`=VALUES(`updated_date`);
                    """;
        }

        protected override string OnDelete(CollectionUnlockKey key)
        {
            return $"""
                UPDATE `collection_unlock` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `user` = {key.User.Escape()} AND `mob_id` = {key.MobId.Escape()} AND `deleted` = 0;
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<CollectionUnlockKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var conditions = keys.Select(k =>
                $"(`user` = {k.User.Escape()} AND `mob_id` = {k.MobId.Escape()})");

            return $"""
                UPDATE `collection_unlock` SET `deleted` = 1, `updated_date` = NOW()
                WHERE ({string.Join(" OR ", conditions)}) AND `deleted` = 0;
                """;
        }
    }
}
