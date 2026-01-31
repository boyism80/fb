using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for user storage box data.
    /// Persists storage entries with Redis hash caching support.
    /// </summary>
    public class StorageBoxRepository : RedisHashRepository<StorageBox, StorageBoxKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="StorageBoxRepository"/> class.
        /// </summary>
        /// <param name="dbContext">Database context for shard connections.</param>
        /// <param name="redisService">Redis service used for caching.</param>
        /// <param name="distributedLock">Distributed lock provider for cache coherence.</param>
        /// <param name="dbExecuteService">Write-back service for async persistence.</param>
        public StorageBoxRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves a specific storage entry for a user.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="user">The user ID.</param>
        /// <param name="id">The storage box ID.</param>
        public Task<StorageBox> Get(uint world, uint user, uint id)
        {
            return base.Get(world, new StorageBoxKey
            {
                User = user,
                Id = id
            });
        }

        /// <summary>
        /// Retrieves all storage entries for a user.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="user">The user ID.</param>
        public Task<IEnumerable<StorageBox>> Get(uint world, uint user)
        {
            return base.GetAll(world, new StorageBoxKey
            {
                User = user
            });
        }

        /// <inheritdoc/>
        protected override string OnSelect(StorageBoxKey key)
        {
            return $"""
                SELECT * FROM `storage_box`
                WHERE `user` = {key.User} AND `id` = {key.Id}
                LIMIT 1;
                """;
        }

        /// <inheritdoc/>
        protected override string OnSelectBulk(StorageBoxKey key)
        {
            return $"""
                SELECT * FROM `storage_box`
                WHERE `user` = {key.User};
                """;
        }

        /// <inheritdoc/>
        protected override string OnUpsert(StorageBox value)
        {
            var attachmentsJson = JsonConvert.SerializeObject(value.Attachments ?? new List<Fb.Model.Dsl>());

            return $"""
                INSERT INTO `storage_box` (
                    `user`,
                    `id`,
                    `title`,
                    `message`,
                    `attachments`,
                    `received`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.Id.Escape()},
                    {value.Title.Escape()},
                    {value.Message.Escape()},
                    {attachmentsJson.Escape()},
                    {value.Received.Escape()},
                    {value.ExpiredDate.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `title`=VALUES(`title`),
                    `message`=VALUES(`message`),
                    `attachments`=VALUES(`attachments`),
                    `received`=VALUES(`received`),
                    `expired_date`=VALUES(`expired_date`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }

        /// <inheritdoc/>
        protected override string OnUpsert(StorageBox[] values)
        {
            var args = values.Select(value =>
            {
                var attachmentsJson = JsonConvert.SerializeObject(value.Attachments ?? new List<Fb.Model.Dsl>());
                return $"""
                        ({value.User.Escape()},
                         {value.Id.Escape()},
                         {value.Title.Escape()},
                         {value.Message.Escape()},
                         {attachmentsJson.Escape()},
                         {value.Received.Escape()},
                         {value.ExpiredDate.Escape()},
                         {value.Deleted.Escape()},
                         {value.CreatedDate.Escape()},
                         {value.UpdatedDate.Escape()})
                        """;
            });

            return $"""
                INSERT INTO `storage_box` (
                    `user`,
                    `id`,
                    `title`,
                    `message`,
                    `attachments`,
                    `received`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `title`=VALUES(`title`),
                    `message`=VALUES(`message`),
                    `attachments`=VALUES(`attachments`),
                    `received`=VALUES(`received`),
                    `expired_date`=VALUES(`expired_date`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}

