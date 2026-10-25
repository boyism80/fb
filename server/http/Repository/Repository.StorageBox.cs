using Dapper;
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

        public async Task<bool> HasLegacyRef(uint world, uint user, string externalRef)
        {
            await using var conn = _dbContext.GetShardConnection(world, user);
            return await conn.ExecuteScalarAsync<int>(
                """
                SELECT COUNT(*) FROM `storage_box_legacy_ref`
                WHERE `user` = @user AND `external_ref` = @externalRef;
                """,
                new { user, externalRef }) > 0;
        }

        public async Task<StorageBox> Get(uint world, uint user, uint id)
        {
            return await base.Get(world, new StorageBoxKey
            {
                User = user,
                Id = id
            });
        }

        public async Task<IEnumerable<StorageBox>> Get(uint world, uint user)
        {
            return await base.GetAll(world, new StorageBoxKey
            {
                User = user
            });
        }

        protected override string OnSelect(StorageBoxKey key)
        {
            return $"""
                SELECT * FROM `storage_box`
                WHERE `user` = {key.User} AND `id` = {key.Id} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(StorageBoxKey key)
        {
            return $"""
                SELECT * FROM `storage_box`
                WHERE `user` = {key.User} AND `deleted` = 0;
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<StorageBoxKey> keys)
        {
            return $"""
                SELECT * FROM `storage_box`
                WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))})
                  AND `deleted` = 0;
                """;
        }

        protected override StorageBoxKey GetKeyFromRow(StorageBox row)
        {
            return new StorageBoxKey { User = row.User };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<StorageBox>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
            {
                return new Dictionary<uint, IReadOnlyList<StorageBox>>();
            }

            var keys = ownerIds.Distinct().Select(uid => new StorageBoxKey { User = uid }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.User, _ => (IList<StorageBox>)new List<StorageBox>());
            foreach (var b in list)
            {
                dict[b.User].Add(b);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<StorageBox>)kv.Value);
        }

        protected override string OnUpsert(StorageBox value)
        {
            var attachmentsJson = JsonConvert.SerializeObject(value.Attachments ?? new List<Fb.Model.Dsl>());

            return $"""
                INSERT INTO `storage_box` (
                    `user`,
                    `id`,
                    `system_storage_box_id`,
                    `external_ref`,
                    `title`,
                    `message`,
                    `attachments`,
                    `received`,
                    `claim_id`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.Id.Escape()},
                    {value.SystemStorageBoxId.Escape()},
                    {value.ExternalRef.Escape()},
                    {value.Title.Escape()},
                    {value.Message.Escape()},
                    {attachmentsJson.Escape()},
                    {value.Received.Escape()},
                    {value.ClaimId.Escape()},
                    {value.ExpiredDate.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `system_storage_box_id`=VALUES(`system_storage_box_id`),
                    `external_ref`=VALUES(`external_ref`),
                    `title`=VALUES(`title`),
                    `message`=VALUES(`message`),
                    `attachments`=VALUES(`attachments`),
                    `received`=VALUES(`received`),
                    `claim_id`=VALUES(`claim_id`),
                    `expired_date`=VALUES(`expired_date`),
                    `deleted`=0,
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
                         {value.SystemStorageBoxId.Escape()},
                         {value.ExternalRef.Escape()},
                         {value.Title.Escape()},
                         {value.Message.Escape()},
                         {attachmentsJson.Escape()},
                         {value.Received.Escape()},
                         {value.ClaimId.Escape()},
                         {value.ExpiredDate.Escape()},
                         0,
                         {value.CreatedDate.Escape()},
                         {value.UpdatedDate.Escape()})
                        """;
            });

            return $"""
                INSERT INTO `storage_box` (
                    `user`,
                    `id`,
                    `system_storage_box_id`,
                    `external_ref`,
                    `title`,
                    `message`,
                    `attachments`,
                    `received`,
                    `claim_id`,
                    `expired_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `system_storage_box_id`=VALUES(`system_storage_box_id`),
                    `external_ref`=VALUES(`external_ref`),
                    `title`=VALUES(`title`),
                    `message`=VALUES(`message`),
                    `attachments`=VALUES(`attachments`),
                    `received`=VALUES(`received`),
                    `claim_id`=VALUES(`claim_id`),
                    `expired_date`=VALUES(`expired_date`),
                    `deleted`=0,
                    `updated_date`=VALUES(`updated_date`);
                """;
        }

        protected override string OnDelete(StorageBoxKey key)
        {
            return $"""
                DELETE FROM `storage_box`
                WHERE `user` = {key.User.Escape()} AND `id` = {key.Id.Escape()};
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<StorageBoxKey> keys)
        {
            if (keys.Count == 0)
            {
                return string.Empty;
            }

            var conditions = keys.Select(k =>
                $"(`user` = {k.User.Escape()} AND `id` = {k.Id.Escape()})");

            return $"""
                DELETE FROM `storage_box`
                WHERE ({string.Join(" OR ", conditions)});
                """;
        }
    }
}
