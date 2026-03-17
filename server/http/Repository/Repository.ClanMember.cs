using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ClanMemberRepository : RedisHashRepository<ClanMember, ClanMemberKey>
    {
        public ClanMemberRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<ClanMember> Get(uint world, uint clan, uint user)
        {
            return await base.Get(world, new ClanMemberKey
            {
                Clan = clan,
                User = user,
            });
        }

        public async Task<IEnumerable<ClanMember>> Get(uint world, uint clan)
        {
            return await base.GetAll(world, new ClanMemberKey
            {
                Clan = clan
            });
        }

        protected override string OnSelect(ClanMemberKey key)
        {
            return $""""
                SELECT * FROM `clan_member` WHERE 
                `clan` = {key.Clan} AND
                `user` = {key.User}
                LIMIT 1;
                """";
        }

        protected override ClanMemberKey GetKeyFromRow(ClanMember row)
        {
            return new ClanMemberKey { Clan = row.Clan, User = 0 };
        }

        protected override string OnSelectBulk(ClanMemberKey key)
        {
            return $"""
                SELECT * FROM `clan_member` WHERE
                `clan` = {key.Clan};
                """;
        }

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
