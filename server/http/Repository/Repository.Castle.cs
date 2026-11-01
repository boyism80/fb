using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class CastleRepository : RedisValueRepository<Castle, CastleKey>
    {
        public CastleRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<Castle> Get(uint world, byte divineBeast)
        {
            return await base.Get(world, new CastleKey { DivineBeast = divineBeast });
        }

        protected override string OnSelect(CastleKey key)
        {
            return $"""
                SELECT * FROM `castle`
                WHERE `divine_beast` = {key.DivineBeast}
                LIMIT 1;
                """;
        }

        protected override CastleKey GetKeyFromRow(Castle row)
        {
            return new CastleKey { DivineBeast = row.DivineBeast };
        }

        protected override string OnUpsert(Castle value)
        {
            var sql = $"""
                INSERT INTO `castle` (
                    `divine_beast`,
                    `owner_clan`,
                    `siege_id`,
                    `siege_key`,
                    `siege_end_at`,
                    `siege_ended`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.DivineBeast.Escape()},
                    {value.OwnerClan.Escape()},
                    {value.SiegeId.Escape()},
                    {value.SiegeKey.Escape()},
                    {value.SiegeEndAt.Escape()},
                    {value.SiegeEnded.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `owner_clan`=VALUES(`owner_clan`),
                    `siege_id`=VALUES(`siege_id`),
                    `siege_key`=VALUES(`siege_key`),
                    `siege_end_at`=VALUES(`siege_end_at`),
                    `siege_ended`=VALUES(`siege_ended`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        protected override string OnDelete(CastleKey key)
        {
            return $"""
                UPDATE `castle` SET `owner_clan` = NULL, `updated_date` = UTC_TIMESTAMP(3)
                WHERE `divine_beast` = {key.DivineBeast.Escape()};
                """;
        }
    }
}
