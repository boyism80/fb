using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ClanEnemyRepository : RedisHashRepository<ClanEnemy, ClanEnemyKey>
    {
        public ClanEnemyRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<ClanEnemy> Get(uint world, uint clan, uint enemyClan)
        {
            return await base.Get(world, new ClanEnemyKey
            {
                Clan = clan,
                EnemyClan = enemyClan,
            });
        }

        public async Task<IEnumerable<ClanEnemy>> Get(uint world, uint clan)
        {
            return await base.GetAll(world, new ClanEnemyKey
            {
                Clan = clan
            });
        }

        protected override string OnSelect(ClanEnemyKey key)
        {
            return $""""
                SELECT * FROM `clan_enemy` WHERE 
                `clan` = {key.Clan} AND
                `enemy_clan` = {key.EnemyClan} AND
                `deleted` = 0
                LIMIT 1;
                """";
        }

        protected override ClanEnemyKey GetKeyFromRow(ClanEnemy row)
        {
            return new ClanEnemyKey { Clan = row.Clan, EnemyClan = 0 };
        }

        protected override string OnSelectBulk(ClanEnemyKey key)
        {
            return $"""
                SELECT * FROM `clan_enemy` WHERE
                `clan` = {key.Clan} AND `deleted` = 0;
                """;
        }

        protected override string OnUpsert(ClanEnemy value)
        {
            var sql = $"""
                INSERT INTO `clan_enemy` (
                    `clan`,
                    `enemy_clan`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Clan.Escape()},
                    {value.EnemyClan.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `deleted` = 0,
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        protected override string OnUpsert(ClanEnemy[] values)
        {
            var args = values.Select(enemy =>
            {
                return $"""
                        ({enemy.Clan.Escape()},
                         {enemy.EnemyClan.Escape()},
                         0,
                         {enemy.CreatedDate.Escape()},
                         {enemy.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO `clan_enemy` (
                        `clan`,
                        `enemy_clan`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `deleted` = 0,
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }

        protected override string OnDelete(ClanEnemyKey key)
        {
            return $"""
                UPDATE `clan_enemy` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `clan` = {key.Clan.Escape()} AND `enemy_clan` = {key.EnemyClan.Escape()} AND `deleted` = 0;
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<ClanEnemyKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var conditions = keys.Select(k =>
                $"(`clan` = {k.Clan.Escape()} AND `enemy_clan` = {k.EnemyClan.Escape()})");

            return $"""
                UPDATE `clan_enemy` SET `deleted` = 1, `updated_date` = NOW()
                WHERE ({string.Join(" OR ", conditions)}) AND `deleted` = 0;
                """;
        }

        public void Delete(uint world, ClanEnemy enemy)
        {
            base.Delete(world, new ClanEnemyKey { Clan = enemy.Clan, EnemyClan = enemy.EnemyClan });
        }
    }
}
