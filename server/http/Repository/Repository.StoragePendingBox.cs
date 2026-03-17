using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;

namespace Http.Reepository
{
    public class StoragePendingBoxRepository : RedisHashRepository<StoragePendingBox, StoragePendingBoxKey>
    {
        public StoragePendingBoxRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<IEnumerable<StoragePendingBox>> Get(uint world, uint? user)
        {
            return base.GetAll(world, new StoragePendingBoxKey
            {
                User = user
            });
        }

        public Task<StoragePendingBox> GetByKey(uint world, StoragePendingBoxKey key)
        {
            return base.Get(world, key);
        }

        protected override string OnSelect(StoragePendingBoxKey key)
        {
            return $"""
                SELECT * FROM `storage_pending_box`
                WHERE `id` = {key.Id.Escape()}
                LIMIT 1;
                """;
        }

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
