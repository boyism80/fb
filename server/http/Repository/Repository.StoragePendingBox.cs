using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for pending storage rewards published by the operation tool.
    /// Implements Redis hash caching for both global and per-user pending queues.
    /// </summary>
    public class StoragePendingBoxRepository : RedisHashRepository<StoragePendingBox, StoragePendingBoxKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="StoragePendingBoxRepository"/> class.
        /// </summary>
        public StoragePendingBoxRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves pending rewards filtered by user scope.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="user">Target user id or null for global rewards.</param>
        public Task<IEnumerable<StoragePendingBox>> Get(uint world, uint? user)
        {
            return base.GetAll(world, new StoragePendingBoxKey
            {
                User = user
            });
        }

        /// <summary>
        /// Retrieves a specific pending reward entry.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="key">The storage pending box key.</param>
        public Task<StoragePendingBox> GetByKey(uint world, StoragePendingBoxKey key)
        {
            return base.Get(world, key);
        }

        /// <inheritdoc/>
        protected override string OnSelect(StoragePendingBoxKey key)
        {
            return $"""
                SELECT * FROM `storage_pending_box`
                WHERE `id` = {key.Id.Escape()}
                LIMIT 1;
                """;
        }

        /// <inheritdoc/>
        protected override string OnSelectBulk(StoragePendingBoxKey key)
        {
            var userPredicate = BuildUserPredicate(key.User);
            return $"""
                SELECT * FROM `storage_pending_box`
                WHERE {userPredicate};
                """;
        }

        private static string BuildUserPredicate(uint? user)
        {
            return user.HasValue ? $"`user` = {user.Value}" : "`user` IS NULL";
        }

        /// <inheritdoc/>
        protected override string OnUpsert(StoragePendingBox value)
        {
            var attachmentsJson = JsonConvert.SerializeObject(value.Attachments ?? new List<Fb.Model.Dsl>());
            return $"""
                INSERT INTO `storage_pending_box` (
                    `id`,
                    `user`,
                    `title`,
                    `message`,
                    `attachments`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {(value.User.HasValue ? value.User.Value.Escape() : "NULL")},
                    {value.Title.Escape()},
                    {value.Message.Escape()},
                    {attachmentsJson.Escape()},
                    {value.ExpiredDate.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `title`=VALUES(`title`),
                    `message`=VALUES(`message`),
                    `attachments`=VALUES(`attachments`),
                    `expired_date`=VALUES(`expired_date`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }

        /// <inheritdoc/>
        protected override string OnUpsert(StoragePendingBox[] values)
        {
            var args = values.Select(value =>
            {
                var attachmentsJson = JsonConvert.SerializeObject(value.Attachments ?? new List<Fb.Model.Dsl>());
                return $"""
                        ({value.Id.Escape()},
                         {(value.User.HasValue ? value.User.Value.Escape() : "NULL")},
                         {value.Title.Escape()},
                         {value.Message.Escape()},
                         {attachmentsJson.Escape()},
                         {value.ExpiredDate.Escape()},
                         {value.Deleted.Escape()},
                         {value.CreatedDate.Escape()},
                         {value.UpdatedDate.Escape()})
                        """;
            });

            return $"""
                INSERT INTO `storage_pending_box` (
                    `id`,
                    `user`,
                    `title`,
                    `message`,
                    `attachments`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `title`=VALUES(`title`),
                    `message`=VALUES(`message`),
                    `attachments`=VALUES(`attachments`),
                    `expired_date`=VALUES(`expired_date`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}

