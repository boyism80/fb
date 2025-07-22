using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for clan member data management.
    /// Implements Redis hash-based caching with database persistence for clan member operations.
    /// </summary>
    public class ClanMemberRepository : RedisHashRepository<ClanMember, ClanMemberKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ClanMemberRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public ClanMemberRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        /// <summary>
        /// Retrieves a specific clan member by clan ID and user ID.
        /// </summary>
        /// <param name="clan">The unique identifier of the clan.</param>
        /// <param name="user">The unique identifier of the user/member.</param>
        /// <returns>The clan member if found; otherwise, null.</returns>
        public async Task<ClanMember> Get(uint clan, uint user)
        {
            return await Get(new ClanMemberKey
            {
                Clan = clan,
                User = user,
            });
        }

        /// <summary>
        /// Retrieves all members of a specific clan by clan ID.
        /// </summary>
        /// <param name="clan">The unique identifier of the clan.</param>
        /// <returns>A collection of all members in the specified clan.</returns>
        public async Task<IEnumerable<ClanMember>> Get(uint clan)
        {
            return await GetAll(new ClanMemberKey
            {
                Clan = clan
            });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a specific clan member.
        /// </summary>
        /// <param name="key">The clan member key containing clan ID and user ID.</param>
        /// <returns>A SQL SELECT statement for the specific clan member.</returns>
        protected override string OnSelect(ClanMemberKey key)
        {
            return $""""
                SELECT * FROM `clan_member` WHERE 
                `clan` = {key.Clan} AND
                `user` = {key.User}
                LIMIT 1;
                """";
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving all members of a clan.
        /// </summary>
        /// <param name="key">The clan member key containing the clan ID.</param>
        /// <returns>A SQL SELECT statement for all members of the specified clan.</returns>
        protected override string OnSelectBulk(ClanMemberKey key)
        {
            return $"""
                SELECT * FROM `clan_member` WHERE
                `clan` = {key.Clan};
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a single clan member.
        /// </summary>
        /// <param name="value">The clan member to upsert.</param>
        /// <returns>A SQL UPSERT statement for the clan member.</returns>
        protected override string OnUpsert(ClanMember value)
        {
            var sql = $"""
                INSERT INTO `clan_member` (
                    `clan`,
                    `user`,
                    `role`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Clan.Escape()},
                    {value.User.Escape()},
                    {value.Role.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `role`=VALUES(`role`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for multiple clan members in a batch operation.
        /// </summary>
        /// <param name="values">The array of clan members to upsert.</param>
        /// <returns>A SQL UPSERT statement for the batch of clan members.</returns>
        protected override string OnUpsert(ClanMember[] values)
        {
            var args = values.Select(member =>
            {
                return $"""
                        ({member.Clan.Escape()},
                         {member.User.Escape()},
                         {member.Role.Escape()},
                         {member.Deleted.Escape()},
                         {member.CreatedDate.Escape()},
                         {member.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO `clan_member` (
                        `clan`,
                        `user`,
                        `role`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `role`=VALUES(`role`),
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}