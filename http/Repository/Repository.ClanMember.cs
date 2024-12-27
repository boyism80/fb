using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ClanMemberRepository : RedisHashRepository<ClanMember, ClanMemberKey>
    {
        public ClanMemberRepository(DbContext dbContext,
            RedisService redisService,
            WriteBackService dbExecuteService) : base(dbContext, redisService, dbExecuteService)
        {
        }

        public async Task<ClanMember> Get(uint clan, uint user)
        {
            return await Get(new ClanMemberKey
            {
                Clan = clan,
                User = user,
            });
        }

        public async Task<IEnumerable<ClanMember>> Get(uint clan)
        {
            return await GetAll(new ClanMemberKey
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
                    `position`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Clan.Escape()},
                    {value.User.Escape()},
                    {value.Position.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `position`=VALUES(`position`),
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
                         {member.Position.Escape()},
                         {member.Deleted.Escape()},
                         {member.CreatedDate.Escape()},
                         {member.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO `clan_member` (
                        `clan`,
                        `user`,
                        `position`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `position`=VALUES(`position`),
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}