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

        protected override string OnSelectMany(IReadOnlyList<StoragePendingBoxKey> keys)
        {
            var users = keys.Select(k => k.User.HasValue ? k.User.Value.ToString() : "NULL").ToList();
            return $"SELECT * FROM `storage_pending_box` WHERE `user` IN ({string.Join(",", users)});";
        }

        protected override StoragePendingBoxKey GetKeyFromRow(StoragePendingBox row)
        {
            return new StoragePendingBoxKey { User = row.User };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<StoragePendingBox>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<StoragePendingBox>>();

            var keys = ownerIds.Distinct().Select(uid => new StoragePendingBoxKey { User = uid == 0 ? null : uid }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.GroupBy(k => k.User ?? 0).ToDictionary(g => g.Key, _ => (IList<StoragePendingBox>)new List<StoragePendingBox>());
            foreach (var p in list)
            {
                dict[p.User ?? 0].Add(p);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<StoragePendingBox>)kv.Value);
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
