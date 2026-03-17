using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;

namespace Http.Reepository
{
    public class StorageBoxRepository : RedisHashRepository<StorageBox, StorageBoxKey>
    {
        public StorageBoxRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<StorageBox> Get(uint world, uint user, uint id)
        {
            return base.Get(world, new StorageBoxKey
            {
                User = user,
                Id = id
            });
        }

        public Task<IEnumerable<StorageBox>> Get(uint world, uint user)
        {
            return base.GetAll(world, new StorageBoxKey
            {
                User = user
            });
        }

        protected override string OnSelect(StorageBoxKey key)
        {
            return $"""
                SELECT * FROM `storage_box`
                WHERE `user` = {key.User} AND `id` = {key.Id}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(StorageBoxKey key)
        {
            return $"""
                SELECT * FROM `storage_box`
                WHERE `user` = {key.User};
                """;
        }

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
