using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ClanAllianceRepository : RedisValueRepository<ClanAlliance, ClanAllianceKey>
    {
        public ClanAllianceRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<ClanAlliance> Get(uint world, uint clan)
        {
            return await base.Get(world, new ClanAllianceKey { Clan = clan });
        }

        protected override string OnSelect(ClanAllianceKey key)
        {
            return $"""
                SELECT * FROM `clan_alliance`
                WHERE `clan` = {key.Clan} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override ClanAllianceKey GetKeyFromRow(ClanAlliance row)
        {
            return new ClanAllianceKey { Clan = row.Clan };
        }

        protected override string OnUpsert(ClanAlliance value)
        {
            var sql = $"""
                INSERT INTO `clan_alliance` (
                    `clan`,
                    `allied_clan`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Clan.Escape()},
                    {value.AlliedClan.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `allied_clan`=VALUES(`allied_clan`),
                    `deleted` = 0,
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        protected override string OnDelete(ClanAllianceKey key)
        {
            return $"""
                UPDATE `clan_alliance` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `clan` = {key.Clan.Escape()} AND `deleted` = 0;
                """;
        }

        public void Delete(uint world, uint clan)
        {
            base.Delete(world, new ClanAllianceKey { Clan = clan });
        }
    }
}
