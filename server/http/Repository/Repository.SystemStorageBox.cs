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

        public async Task<List<SystemStorageBox>> GetAll(uint world, uint user)
        {
            var all = (await base.GetAll(world, new SystemStorageBoxKey { Id = 0 }))
                .Concat(await base.GetAll(world, new SystemStorageBoxKey { Id = 0, User = user }));
            return all.OrderByDescending(b => b.Id).ToList();
        }

        public async Task<List<uint>> GetArchived(uint world, IReadOnlyCollection<uint> ids)
        {
            if (ids.Count == 0)
            {
                return new List<uint>();
            }

            await using var conn = _dbContext.GetGlobalConnection(world);
            var rows = await conn.QueryAsync<uint>(
                "SELECT `id` FROM `system_storage_box_archive` WHERE `id` IN @ids;",
                new { ids });
            return rows.ToList();
        }

        public async Task<List<(uint Id, uint User, string ExternalRef)>> GetArchivedMarketplace(uint world, uint afterId, int limit)
        {
            await using var conn = _dbContext.GetGlobalConnection(world);
            var rows = await conn.QueryAsync<(uint Id, uint User, string ExternalRef)>(
                """
                SELECT `id` AS Id, `user` AS User, `external_ref` AS ExternalRef FROM `system_storage_box_archive`
                WHERE `id` > @afterId AND `user` IS NOT NULL AND `external_ref` LIKE 'marketplace:%'
                ORDER BY `id`
                LIMIT @limit;
                """,
                new { afterId, limit });
            return rows.ToList();
        }

        public async Task DeleteArchived(uint world, uint id)
        {
            await using var conn = _dbContext.GetGlobalConnection(world);
            await conn.ExecuteAsync("DELETE FROM `system_storage_box_archive` WHERE `id` = @id;", new { id });
        }

        public async Task<List<(uint Id, uint User)>> GetRecipients(uint world)
        {
            await using var conn = _dbContext.GetGlobalConnection(world);
            var rows = await conn.QueryAsync<(uint Id, uint User)>(
                """
                SELECT `id` AS Id, `user` AS User FROM `system_storage_box`
                WHERE `user` IS NOT NULL AND `deleted` = 0;
                """);
            return rows.ToList();
        }

        public async Task<SystemStorageBox> GetByExternalRef(uint world, string externalRef)
        {
            if (string.IsNullOrWhiteSpace(externalRef))
            {
                return null;
            }

            await using var conn = _dbContext.GetGlobalConnection(world);
            var row = await conn.QueryFirstOrDefaultAsync<SystemStorageBox>(
                """
                SELECT * FROM `system_storage_box`
                WHERE `external_ref` = @externalRef AND `deleted` = 0
                LIMIT 1;
                """,
                new { externalRef });

            if (row != null)
            {
                return row;
            }

            // A resend after the box was archived must not create a second box.
            return await conn.QueryFirstOrDefaultAsync<SystemStorageBox>(
                """
                SELECT * FROM `system_storage_box_archive`
                WHERE `external_ref` = @externalRef
                LIMIT 1;
                """,
                new { externalRef });
        }

        // Every box that is not archived yet, including reserved and canceled ones.
        public async Task<List<SystemStorageBox>> GetAll(uint world)
        {
            var all = await base.GetAll(world, new SystemStorageBoxKey { Id = 0 });
            return all.OrderByDescending(b => b.Id).ToList();
        }

        public async Task<List<SystemStorageBox>> GetActive(uint world)
        {
            var now = DateTime.UtcNow;
            var all = await base.GetAll(world, new SystemStorageBoxKey { Id = 0 });
            return all.Where(b => b.Active(now))
                .OrderByDescending(b => b.Id)
                .ToList();
        }

        protected override string OnSelect(SystemStorageBoxKey key)
        {
            return $"""
                SELECT * FROM `system_storage_box`
                WHERE `id` = {key.Id} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override SystemStorageBoxKey GetKeyFromRow(SystemStorageBox row)
        {
            return new SystemStorageBoxKey { Id = 0, User = row.User };
        }

        protected override string OnSelectBulk(SystemStorageBoxKey key)
        {
            var user = key.User == null ? "IS NULL" : $"= {key.User.Value}";
            return $"""
                SELECT * FROM `system_storage_box` WHERE `deleted` = 0 AND `user` {user};
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
                    `reserve_date`,
                    `expired_date`,
                    `include_new_characters`,
                    `canceled`,
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
                    {value.ReserveDate.Escape()},
                    {value.ExpiredDate.Escape()},
                    {value.IncludeNewCharacters.Escape()},
                    {value.Canceled.Escape()},
                    {value.ExternalRef.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `title`=VALUES(`title`),
                    `message`=VALUES(`message`),
                    `attachments`=VALUES(`attachments`),
                    `reserve_date`=VALUES(`reserve_date`),
                    `expired_date`=VALUES(`expired_date`),
                    `include_new_characters`=VALUES(`include_new_characters`),
                    `canceled`=VALUES(`canceled`),
                    `external_ref`=VALUES(`external_ref`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }

        protected override string OnDelete(SystemStorageBoxKey key)
        {
            return OnDeleteMany(new[] { key });
        }

        // Rows move to the archive through the write-back queue, after any upsert queued before them.
        protected override string OnDeleteMany(IReadOnlyList<SystemStorageBoxKey> keys)
        {
            var ids = string.Join(", ", keys.Select(k => k.Id));
            return $"""
                INSERT IGNORE INTO `system_storage_box_archive` (
                    `id`, `user`, `title`, `message`, `attachments`, `reserve_date`, `expired_date`,
                    `include_new_characters`, `canceled`, `external_ref`, `created_date`, `updated_date`)
                SELECT `id`, `user`, `title`, `message`, `attachments`, `reserve_date`, `expired_date`,
                    `include_new_characters`, `canceled`, `external_ref`, `created_date`, `updated_date`
                FROM `system_storage_box` WHERE `id` IN ({ids});
                DELETE FROM `system_storage_box` WHERE `id` IN ({ids});
                """;
        }

        // Returns false when the box is gone or its reservation is too close to edit.
        public async Task<bool> Edit(uint world, uint id, string title, string message, List<Fb.Model.Dsl> attachments, DateTime reserveDate, DateTime? expireDate, bool includeNewCharacters)
        {
            var box = await Get(world, id);
            var deadline = DateTime.UtcNow + SystemMailRepository.EditLeadTime;
            if (box == null || box.Canceled || box.ReserveDate == null || box.ReserveDate <= deadline || reserveDate <= deadline)
            {
                return false;
            }

            box.Title = title;
            box.Message = message;
            box.Attachments = attachments;
            box.ReserveDate = reserveDate;
            box.ExpiredDate = expireDate;
            box.IncludeNewCharacters = includeNewCharacters;
            Set(world, box);
            await SaveChangesAsync();
            return true;
        }

        public async Task<bool> Cancel(uint world, uint id)
        {
            var box = await Get(world, id);
            if (box == null || box.Canceled || box.ReserveDate == null || box.ReserveDate <= DateTime.UtcNow + SystemMailRepository.EditLeadTime)
            {
                return false;
            }

            box.Canceled = true;
            Set(world, box);
            Delete(world, (SystemStorageBoxKey)box);
            await SaveChangesAsync();
            return true;
        }

        public async Task Archive(uint world, IReadOnlyList<SystemStorageBox> boxes)
        {
            if (boxes.Count == 0)
            {
                return;
            }

            Delete(world, boxes);
            await SaveChangesAsync();
        }

        public async Task<SystemStorageBox> Write(uint world,
            string title,
            string message,
            List<Fb.Model.Dsl> attachments,
            DateTime? reserveDate,
            DateTime? expireDate,
            bool includeNewCharacters,
            string externalRef,
            uint? user)
        {
            if (!string.IsNullOrWhiteSpace(externalRef))
            {
                var existing = await GetByExternalRef(world, externalRef);
                if (existing != null)
                {
                    return existing;
                }
            }

            var attachmentsJson = JsonConvert.SerializeObject(attachments ?? new List<Fb.Model.Dsl>());

            var query = """
                INSERT INTO system_storage_box (user, title, message, attachments, reserve_date, expired_date, include_new_characters, canceled, external_ref, deleted, created_date, updated_date)
                VALUES (@user, @title, @message, CAST(@attachmentsJson AS JSON), @reserveDate, @expireDate, @includeNewCharacters, 0, @externalRef, 0, UTC_TIMESTAMP(3), UTC_TIMESTAMP(3));
                SELECT LAST_INSERT_ID();
                """;

            await using var conn = _dbContext.GetGlobalConnection(world);
            var id = await conn.QueryFirstOrDefaultAsync<uint>(query, new
            {
                user,
                title,
                message,
                attachmentsJson,
                reserveDate,
                expireDate,
                includeNewCharacters,
                externalRef
            });

            var box = new SystemStorageBox
            {
                Id = id,
                User = user,
                Title = title,
                Message = message,
                Attachments = attachments ?? new List<Fb.Model.Dsl>(),
                ReserveDate = reserveDate,
                ExpiredDate = expireDate,
                IncludeNewCharacters = includeNewCharacters,
                ExternalRef = externalRef,
                CreatedDate = DateTime.UtcNow,
                UpdatedDate = DateTime.UtcNow
            };

            Set(world, box);
            await SaveChangesAsync();

            return box;
        }
    }
}
