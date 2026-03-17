using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;
using System.Data;

namespace Http.Reepository
{
    public class SystemMailRepository : RedisHashRepository<SystemMail, SystemMailKey>
    {
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

        public async Task<List<SystemMail>> GetAll(uint world)
        {
            // Use a dummy key to get all system mails from the same hash
            var allMails = await base.GetAll(world, new SystemMailKey { Id = 0 });
            var now = DateTime.Now;
            return allMails
                .Where(m => !m.Deleted && (m.ExpireDate == null || m.ExpireDate > now))
                .OrderByDescending(m => m.Id)
                .ToList();
        }

        protected override string OnSelect(SystemMailKey key)
        {
            return $"""
                SELECT * FROM `system_mail` WHERE 
                `id` = {key.Id}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(SystemMailKey key)
        {
            return """
                SELECT * FROM `system_mail`;
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
                    `expire_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Sender.Escape()},
                    {value.Title.Escape()},
                    {value.Contents.Escape()},
                    {value.ExpireDate.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `sender`=VALUES(`sender`),
                    `title`=VALUES(`title`), 
                    `contents`=VALUES(`contents`), 
                    `expire_date`=VALUES(`expire_date`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        public async Task<SystemMail> Write(uint world, uint sender, string title, string contents, DateTime? expireDate)
        {
            var query = @"
                INSERT INTO system_mail (sender, title, contents, expire_date, deleted, created_date, updated_date)
                VALUES (@sender, @title, @contents, @expireDate, 0, NOW(), NOW());
                SELECT LAST_INSERT_ID();";

            // SystemMail uses world-global database
            await using var conn = _dbContext.GetGlobalConnection(world);
            var id = await conn.QueryFirstOrDefaultAsync<uint>(query, new { sender, title, contents, expireDate });

            var systemMail = new SystemMail
            {
                Id = id,
                Sender = sender,
                Title = title,
                Contents = contents,
                ExpireDate = expireDate,
                CreatedDate = DateTime.Now,
                UpdatedDate = DateTime.Now,
                Deleted = false
            };

            // Update Redis cache using the caching system
            Set(world, systemMail);
            await SaveChangesAsync();

            return systemMail;
        }
    }
}
