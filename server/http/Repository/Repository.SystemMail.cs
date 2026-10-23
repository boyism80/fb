using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;
using System.Data;

namespace Http.Reepository
{
    public class SystemMailRepository : RedisHashRepository<SystemMail, SystemMailKey>
    {
        // Checking a reservation and saving the edit are separate steps, so an edit this close to the send time
        // could reach Redis after a game server has already taken the mail.
        public static readonly TimeSpan EditLeadTime = TimeSpan.FromSeconds(10);

        public SystemMailRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<SystemMail> Get(uint world, uint id)
        {
            return await base.Get(world, new SystemMailKey { Id = id });
        }

        // Every mail that is not archived yet, including reserved and canceled ones.
        public async Task<List<SystemMail>> GetAll(uint world)
        {
            var mails = await base.GetAll(world, new SystemMailKey { Id = 0 });
            return mails.OrderByDescending(m => m.Id).ToList();
        }

        public async Task<List<SystemMail>> GetActive(uint world)
        {
            var now = DateTime.UtcNow;
            var mails = await base.GetAll(world, new SystemMailKey { Id = 0 });
            return mails.Where(m => m.Active(now))
                .OrderByDescending(m => m.Id)
                .ToList();
        }

        protected override string OnSelect(SystemMailKey key)
        {
            return $"""
                SELECT * FROM `system_mail` WHERE 
                `id` = {key.Id} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override SystemMailKey GetKeyFromRow(SystemMail row)
        {
            return new SystemMailKey { Id = 0 };
        }

        protected override string OnSelectBulk(SystemMailKey key)
        {
            return """
                SELECT * FROM `system_mail` WHERE `deleted` = 0;
                """;
        }

        protected override string OnUpsert(SystemMail value)
        {
            var sql = $"""
                INSERT INTO `system_mail` (
                    `id`,
                    `sender`,
                    `title`,
                    `contents`,
                    `reserve_date`,
                    `expire_date`,
                    `include_new_characters`,
                    `canceled`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Sender.Escape()},
                    {value.Title.Escape()},
                    {value.Contents.Escape()},
                    {value.ReserveDate.Escape()},
                    {value.ExpireDate.Escape()},
                    {value.IncludeNewCharacters.Escape()},
                    {value.Canceled.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `sender`=VALUES(`sender`),
                    `title`=VALUES(`title`), 
                    `contents`=VALUES(`contents`), 
                    `reserve_date`=VALUES(`reserve_date`),
                    `expire_date`=VALUES(`expire_date`),
                    `include_new_characters`=VALUES(`include_new_characters`),
                    `canceled`=VALUES(`canceled`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        protected override string OnDelete(SystemMailKey key)
        {
            return OnDeleteMany(new[] { key });
        }

        // Rows move to the archive through the write-back queue, after any upsert queued before them.
        protected override string OnDeleteMany(IReadOnlyList<SystemMailKey> keys)
        {
            var ids = string.Join(", ", keys.Select(k => k.Id));
            return $"""
                INSERT IGNORE INTO `system_mail_archive` (
                    `id`, `sender`, `title`, `contents`, `reserve_date`, `expire_date`,
                    `include_new_characters`, `canceled`, `created_date`, `updated_date`)
                SELECT `id`, `sender`, `title`, `contents`, `reserve_date`, `expire_date`,
                    `include_new_characters`, `canceled`, `created_date`, `updated_date`
                FROM `system_mail` WHERE `id` IN ({ids});
                DELETE FROM `system_mail` WHERE `id` IN ({ids});
                """;
        }

        public async Task<SystemMail> Write(uint world, uint sender, string title, string contents, DateTime? reserveDate, DateTime? expireDate, bool includeNewCharacters)
        {
            var query = @"
                INSERT INTO system_mail (sender, title, contents, reserve_date, expire_date, include_new_characters, canceled, deleted, created_date, updated_date)
                VALUES (@sender, @title, @contents, @reserveDate, @expireDate, @includeNewCharacters, 0, 0, UTC_TIMESTAMP(3), UTC_TIMESTAMP(3));
                SELECT LAST_INSERT_ID();";

            // SystemMail uses world-global database
            await using var conn = _dbContext.GetGlobalConnection(world);
            var id = await conn.QueryFirstOrDefaultAsync<uint>(query, new { sender, title, contents, reserveDate, expireDate, includeNewCharacters });

            var systemMail = new SystemMail
            {
                Id = id,
                Sender = sender,
                Title = title,
                Contents = contents,
                ReserveDate = reserveDate,
                ExpireDate = expireDate,
                IncludeNewCharacters = includeNewCharacters,
                CreatedDate = DateTime.UtcNow,
                UpdatedDate = DateTime.UtcNow
            };

            // Update Redis cache using the caching system
            Set(world, systemMail);
            await SaveChangesAsync();

            return systemMail;
        }

        // Returns false when the mail is gone or its reservation is too close to edit.
        public async Task<bool> Edit(uint world, uint id, string title, string contents, DateTime reserveDate, DateTime? expireDate, bool includeNewCharacters)
        {
            var mail = await Get(world, id);
            var deadline = DateTime.UtcNow + EditLeadTime;
            if (mail == null || mail.Canceled || mail.ReserveDate == null || mail.ReserveDate <= deadline || reserveDate <= deadline)
            {
                return false;
            }

            mail.Title = title;
            mail.Contents = contents;
            mail.ReserveDate = reserveDate;
            mail.ExpireDate = expireDate;
            mail.IncludeNewCharacters = includeNewCharacters;
            Set(world, mail);
            await SaveChangesAsync();
            return true;
        }

        public async Task<bool> Cancel(uint world, uint id)
        {
            var mail = await Get(world, id);
            if (mail == null || mail.Canceled || mail.ReserveDate == null || mail.ReserveDate <= DateTime.UtcNow + EditLeadTime)
            {
                return false;
            }

            mail.Canceled = true;
            Set(world, mail);
            Delete(world, (SystemMailKey)mail);
            await SaveChangesAsync();
            return true;
        }

        public async Task Archive(uint world, IReadOnlyList<SystemMail> mails)
        {
            if (mails.Count == 0)
            {
                return;
            }

            Delete(world, mails);
            await SaveChangesAsync();
        }
    }
}
