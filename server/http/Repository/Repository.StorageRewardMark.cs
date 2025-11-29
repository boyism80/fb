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

        public Task<IEnumerable<StorageRewardMark>> Get(uint user)
        {
            return base.GetAll(new StorageRewardMarkKey
            {
                User = user
            });
        }

        public Task<StorageRewardMark> Get(uint user, string pendingId)
        {
            return base.Get(new StorageRewardMarkKey
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

