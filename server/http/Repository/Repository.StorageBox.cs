using System.Collections.Generic;
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
        public Task<StorageBox> Get(uint user, uint id)
        {
            return base.Get(new StorageBoxKey
            {
                User = user,
                Id = id
            });
        }

        /// <summary>
        /// Retrieves all storage entries for a user.
        /// </summary>
        public Task<IEnumerable<StorageBox>> Get(uint user)
        {
            return base.GetAll(new StorageBoxKey
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
                    {value.Message.Escape()},
                    {attachmentsJson.Escape()},
                    {value.Received.Escape()},
                    {value.ExpiredDate.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
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
                    `message`,
                    `attachments`,
                    `received`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
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

