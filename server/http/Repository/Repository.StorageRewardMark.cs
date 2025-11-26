using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for storage reward mark tracking.
    /// Ensures pending rewards are processed only once per user.
    /// </summary>
    public class StorageRewardMarkRepository : RedisHashRepository<StorageRewardMark, StorageRewardMarkKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="StorageRewardMarkRepository"/> class.
        /// </summary>
        public StorageRewardMarkRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves all reward marks for a user.
        /// </summary>
        public Task<IEnumerable<StorageRewardMark>> Get(uint user)
        {
            return base.GetAll(new StorageRewardMarkKey
            {
                User = user
            });
        }

        /// <summary>
        /// Retrieves a specific reward mark.
        /// </summary>
        public Task<StorageRewardMark> Get(uint user, ulong pendingId)
        {
            return base.Get(new StorageRewardMarkKey
            {
                User = user,
                PendingId = pendingId
            });
        }

        /// <inheritdoc/>
        protected override string OnSelect(StorageRewardMarkKey key)
        {
            return $"""
                SELECT * FROM `storage_reward_mark`
                WHERE `pending_id` = {key.PendingId}
                LIMIT 1;
                """;
        }

        /// <inheritdoc/>
        protected override string OnSelectBulk(StorageRewardMarkKey key)
        {
            return $"""
                SELECT * FROM `storage_reward_mark`
                WHERE `user` = {key.User};
                """;
        }

        /// <inheritdoc/>
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

        /// <inheritdoc/>
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

