using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;
using System.Data;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for system mail data management.
    /// Implements Redis hash-based caching with database persistence for system mail operations.
    /// </summary>
    public class SystemMailRepository : RedisHashRepository<SystemMail, SystemMailKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SystemMailRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public SystemMailRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves a specific system mail by section and its ID.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="id">The unique identifier of the system mail.</param>
        /// <returns>The system mail if found; otherwise, null.</returns>
        public async Task<SystemMail> Get(string section, uint id)
        {
            return await base.Get(section, new SystemMailKey { Id = id });
        }

        /// <summary>
        /// Retrieves all active (non-expired and non-deleted) system mails for the specified section.
        /// Filters out expired mails based on current time.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <returns>A list of active system mails that have not expired.</returns>
        public async Task<List<SystemMail>> GetAll(string section)
        {
            // Use a dummy key to get all system mails from the same hash
            var allMails = await base.GetAll(section, new SystemMailKey { Id = 0 });
            var now = DateTime.Now;
            return allMails
                .Where(m => !m.Deleted && (m.ExpireDate == null || m.ExpireDate > now))
                .OrderByDescending(m => m.Id)
                .ToList();
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a specific system mail.
        /// </summary>
        /// <param name="key">The system mail key containing the mail ID.</param>
        /// <returns>A SQL SELECT statement for the specific system mail.</returns>
        protected override string OnSelect(SystemMailKey key)
        {
            return $"""
                SELECT * FROM `system_mail` WHERE 
                `id` = {key.Id}
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving all system mails.
        /// </summary>
        /// <param name="key">The system mail key (unused, but required by base class).</param>
        /// <returns>A SQL SELECT statement for all system mails.</returns>
        protected override string OnSelectBulk(SystemMailKey key)
        {
            return """
                SELECT * FROM `system_mail`;
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a single system mail.
        /// </summary>
        /// <param name="value">The system mail to upsert.</param>
        /// <returns>A SQL UPSERT statement for the system mail.</returns>
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

        /// <summary>
        /// Creates a new system mail and queues it for database write-back.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="sender">The sender's user ID for the system mail.</param>
        /// <param name="title">The title/subject of the system mail.</param>
        /// <param name="contents">The body content of the system mail.</param>
        /// <param name="expireDate">Optional expiration date for the system mail.</param>
        /// <returns>The created system mail with assigned ID.</returns>
        public async Task<SystemMail> Write(string section, uint sender, string title, string contents, DateTime? expireDate)
        {
            var query = @"
                INSERT INTO system_mail (sender, title, contents, expire_date, deleted, created_date, updated_date)
                VALUES (@sender, @title, @contents, @expireDate, 0, NOW(), NOW());
                SELECT LAST_INSERT_ID();";

            // SystemMail uses section-global database
            await using var conn = _dbContext.Connection(section, null);
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
            Set(section, systemMail);
            await SaveChangesAsync();

            return systemMail;
        }
    }
}

