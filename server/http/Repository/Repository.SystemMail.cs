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
        /// Retrieves a specific system mail by its ID.
        /// </summary>
        /// <param name="id">The unique identifier of the system mail.</param>
        /// <returns>The system mail if found; otherwise, null.</returns>
        public async Task<SystemMail> Get(uint id)
        {
            return await Get(new SystemMailKey { Id = id });
        }

        /// <summary>
        /// Retrieves all active (non-expired and non-deleted) system mails.
        /// Filters out expired mails based on current time.
        /// </summary>
        /// <returns>A list of active system mails that have not expired.</returns>
        public async Task<List<SystemMail>> GetAll()
        {
            // Use a dummy key to get all system mails from the same hash
            var allMails = await GetAll(new SystemMailKey { Id = 0 });
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
                    `title`,
                    `contents`,
                    `expire_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Title.Escape()},
                    {value.Contents.Escape()},
                    {value.ExpireDate.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
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
        /// <param name="title">The title/subject of the system mail.</param>
        /// <param name="contents">The body content of the system mail.</param>
        /// <param name="expireDate">Optional expiration date for the system mail.</param>
        /// <returns>The created system mail with assigned ID.</returns>
        public async Task<SystemMail> Write(string title, string contents, DateTime? expireDate)
        {
            var query = @"
                INSERT INTO system_mail (title, contents, expire_date, deleted, created_date, updated_date)
                VALUES (@title, @contents, @expireDate, 0, NOW(), NOW());
                SELECT LAST_INSERT_ID();";

            await using var conn = _dbContext.Connection(0);
            var id = await conn.QueryFirstOrDefaultAsync<uint>(query, new { title, contents, expireDate });

            var systemMail = new SystemMail
            {
                Id = id,
                Title = title,
                Contents = contents,
                ExpireDate = expireDate,
                CreatedDate = DateTime.Now,
                UpdatedDate = DateTime.Now,
                Deleted = false
            };

            // Queue for write-back to update Redis cache
            await Upsert(systemMail);

            return systemMail;
        }

        /// <summary>
        /// Retrieves the list of system mail IDs that a user has not yet received.
        /// Uses Redis hash cache for fast lookup of received system mails.
        /// </summary>
        /// <param name="user">The unique identifier of the user.</param>
        /// <returns>A list of system mail IDs that the user has not received.</returns>
        public async Task<List<uint>> GetUnreceivedSystemMails(uint user)
        {
            // Get all active system mails
            var allSystemMails = await GetAll();
            if (allSystemMails.Count == 0)
                return new List<uint>();

            // Get system mails that the user has already received (from Redis hash cache)
            var receivedSystemMails = await _dbContext.SystemMailUser.Get(user);
            var receivedMailIds = receivedSystemMails
                .Where(smu => !smu.Deleted)
                .Select(smu => smu.MailId)
                .ToHashSet();

            // Return system mail IDs that the user has not received
            return allSystemMails
                .Where(m => !receivedMailIds.Contains(m.Id))
                .Select(m => m.Id)
                .ToList();
        }

        /// <summary>
        /// Sends a system mail to a specific user by copying it to the mail table.
        /// Creates a mail entry in the mail table and records it in system_mail_user to prevent duplicates.
        /// Uses sender "System" to indicate system mail.
        /// </summary>
        /// <param name="user">The unique identifier of the user to send the system mail to.</param>
        /// <param name="systemMailId">The unique identifier of the system mail to send.</param>
        /// <returns>The created mail entry, or null if the system mail does not exist or has already been sent.</returns>
        public async Task<Mail?> SendSystemMailToUser(uint user, uint systemMailId)
        {
            // Check if system mail exists and is active
            var systemMail = await Get(systemMailId);
            if (systemMail == null || systemMail.Deleted)
                return null;

            // Check if already sent (prevent duplicates) - use Redis hash cache
            var existing = await _dbContext.SystemMailUser.Get(user, systemMailId);
            if (existing != null && !existing.Deleted)
                return null;

            // Check expiration
            if (systemMail.ExpireDate.HasValue && systemMail.ExpireDate.Value < DateTime.Now)
                return null;

            // Use stored procedure to create mail (same as regular mail)
            await using var conn = _dbContext.Connection(user);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("user", user);
            dynamicParams.Add("sender", "System");
            dynamicParams.Add("title", systemMail.Title);
            dynamicParams.Add("contents", systemMail.Contents);

            await using var reader = await conn.QueryMultipleAsync(
                "USP_MAIL_WRITE",
                dynamicParams,
                commandType: CommandType.StoredProcedure);

            var success = await reader.ReadFirstAsync<bool>();
            if (!success)
                return null;

            var mail = await reader.ReadFirstOrDefaultAsync<Mail>();
            if (mail == null)
                return null;

            // Record in system_mail_user to prevent duplicates (uses Redis hash cache with write-back)
            var systemMailUser = new SystemMailUser
            {
                User = user,
                MailId = systemMailId,
                Read = false,
                Deleted = false,
                CreatedDate = DateTime.Now
            };
            await _dbContext.SystemMailUser.Upsert(systemMailUser);

            return mail;
        }
    }
}

