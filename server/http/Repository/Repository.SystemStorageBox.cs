using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;
using System.Data;

namespace Http.Reepository
{
    public class SystemStorageBoxRepository : RedisHashRepository<SystemStorageBox, SystemStorageBoxKey>
    {
        public SystemStorageBoxRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<SystemStorageBox> Get(uint world, uint id)
        {
            return await base.Get(world, new SystemStorageBoxKey { Id = id });
        }

        public async Task<SystemStorageBox> GetByExternalRef(uint world, string externalRef)
        {
            if (string.IsNullOrWhiteSpace(externalRef))
                return null;

            await using var conn = _dbContext.GetGlobalConnection(world);
            var row = await conn.QueryFirstOrDefaultAsync<SystemStorageBox>(
                """
                SELECT * FROM `system_storage_box`
                WHERE `external_ref` = @externalRef AND `deleted` = 0
                LIMIT 1;
                """,
                new { externalRef });

            if (row != null)
                Set(world, row);

            return row;
        }

        public async Task<List<SystemStorageBox>> GetAll(uint world, uint offset = 0)
        {
            var all = await base.GetAll(world, new SystemStorageBoxKey { Id = 0 });
            var now = DateTime.Now;
            return all
                .Where(b => b.Id >= offset
                            && !b.Deleted
                            && (b.ExpiredDate == null || b.ExpiredDate > now))
                .OrderByDescending(b => b.Id)
                .ToList();
        }

        protected override string OnSelect(SystemStorageBoxKey key)
        {
            return $"""
                SELECT * FROM `system_storage_box`
                WHERE `id` = {key.Id}
                LIMIT 1;
                """;
        }

        protected override SystemStorageBoxKey GetKeyFromRow(SystemStorageBox row)
        {
            return new SystemStorageBoxKey { Id = 0 };
        }

        protected override string OnSelectBulk(SystemStorageBoxKey key)
        {
            return """
                SELECT * FROM `system_storage_box`;
                """;
        }

        protected override string OnUpsert(SystemStorageBox value)
        {
            var attachmentsJson = JsonConvert.SerializeObject(value.Attachments ?? new List<Fb.Model.Dsl>());

            return $"""
                INSERT INTO `system_storage_box` (
                    `id`,
                    `user`,
                    `title`,
                    `message`,
                    `attachments`,
                    `expired_date`,
                    `external_ref`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.User.Escape()},
                    {value.Title.Escape()},
                    {value.Message.Escape()},
                    {attachmentsJson.Escape()},
                    {value.ExpiredDate.Escape()},
                    {value.ExternalRef.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `user`=VALUES(`user`),
                    `title`=VALUES(`title`),
                    `message`=VALUES(`message`),
                    `attachments`=VALUES(`attachments`),
                    `expired_date`=VALUES(`expired_date`),
                    `external_ref`=VALUES(`external_ref`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }

        public async Task<SystemStorageBox> Write(uint world,
            uint? user,
            string title,
            string message,
            List<Fb.Model.Dsl> attachments,
            DateTime? expireDate,
            string externalRef)
        {
            if (!string.IsNullOrWhiteSpace(externalRef))
            {
                var existing = await GetByExternalRef(world, externalRef);
                if (existing != null)
                    return existing;
            }

            var attachmentsJson = JsonConvert.SerializeObject(attachments ?? new List<Fb.Model.Dsl>());

            var query = """
                INSERT INTO system_storage_box (`user`, title, message, attachments, expired_date, external_ref, deleted, created_date, updated_date)
                VALUES (@user, @title, @message, CAST(@attachmentsJson AS JSON), @expireDate, @externalRef, 0, NOW(), NOW());
                SELECT LAST_INSERT_ID();
                """;

            await using var conn = _dbContext.GetGlobalConnection(world);
            var id = await conn.QueryFirstOrDefaultAsync<uint>(query, new
            {
                user,
                title,
                message,
                attachmentsJson,
                expireDate,
                externalRef
            });

            var box = new SystemStorageBox
            {
                Id           = id,
                User         = user,
                Title        = title,
                Message      = message,
                Attachments  = attachments ?? new List<Fb.Model.Dsl>(),
                ExpiredDate  = expireDate,
                ExternalRef  = externalRef,
                CreatedDate  = DateTime.Now,
                UpdatedDate  = DateTime.Now,
                Deleted      = false
            };

            Set(world, box);
            await SaveChangesAsync();

            return box;
        }
    }
}
