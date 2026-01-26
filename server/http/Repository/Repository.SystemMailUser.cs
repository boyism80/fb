using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for system mail user data management.
    /// Implements Redis hash-based caching with database persistence for system mail user operations.
    /// </summary>
    public class SystemMailUserRepository : RedisHashRepository<SystemMailUser, SystemMailUserKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SystemMailUserRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public SystemMailUserRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves a specific system mail user record by section, user ID and mail ID.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="user">The unique identifier of the user.</param>
        /// <param name="mailId">The unique identifier of the system mail.</param>
        /// <returns>The system mail user record if found; otherwise, null.</returns>
        public async Task<SystemMailUser> Get(string section, uint user, uint mailId)
        {
            return await base.Get(section, new SystemMailUserKey
            {
                User = user,
                MailId = mailId
            });
        }

        /// <summary>
        /// Retrieves all system mail user records for a specific user.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="user">The unique identifier of the user.</param>
        /// <returns>A collection of all system mail user records for the specified user.</returns>
        public async Task<IEnumerable<SystemMailUser>> Get(string section, uint user)
        {
            return await base.GetAll(section, new SystemMailUserKey
            {
                User = user,
                MailId = 0
            });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a specific system mail user record.
        /// </summary>
        /// <param name="key">The system mail user key containing user ID and mail ID.</param>
        /// <returns>A SQL SELECT statement for the specific system mail user record.</returns>
        protected override string OnSelect(SystemMailUserKey key)
        {
            return $"""
                SELECT * FROM `system_mail_user` WHERE 
                `user` = {key.User} AND
                `mail_id` = {key.MailId}
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving all system mail user records for a user.
        /// </summary>
        /// <param name="key">The system mail user key containing the user ID.</param>
        /// <returns>A SQL SELECT statement for all system mail user records of the specified user.</returns>
        protected override string OnSelectBulk(SystemMailUserKey key)
        {
            return $"""
                SELECT * FROM `system_mail_user` WHERE
                `user` = {key.User};
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a single system mail user record.
        /// </summary>
        /// <param name="value">The system mail user record to upsert.</param>
        /// <returns>A SQL UPSERT statement for the system mail user record.</returns>
        protected override string OnUpsert(SystemMailUser value)
        {
            var sql = $"""
                INSERT INTO `system_mail_user` (
                    `mail_id`,
                    `user`,
                    `read`,
                    `deleted`,
                    `created_date`)
                VALUES (
                    {value.MailId.Escape()},
                    {value.User.Escape()},
                    {value.Read.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `read`=VALUES(`read`), 
                    `deleted`=VALUES(`deleted`);
                """;

            return sql;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for multiple system mail user records in a batch operation.
        /// </summary>
        /// <param name="values">The array of system mail user records to upsert.</param>
        /// <returns>A SQL UPSERT statement for the batch of system mail user records.</returns>
        protected override string OnUpsert(SystemMailUser[] values)
        {
            var args = values.Select(smu =>
            {
                return $"""
                        ({smu.MailId.Escape()},
                         {smu.User.Escape()},
                         {smu.Read.Escape()},
                         {smu.Deleted.Escape()},
                         {smu.CreatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO system_mail_user (
                        `mail_id`,
                        `user`,
                        `read`,
                        `deleted`,
                        `created_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `read`=VALUES(`read`),
                        `deleted`=VALUES(`deleted`);
                    """;

            return sql;
        }
    }
}
