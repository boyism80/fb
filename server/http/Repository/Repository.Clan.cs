using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ClanRepository : RedisValueRepository<Clan, ClanKey>
    {
        public ClanRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<Clan> Get(uint world, uint id)
        {
            return await base.Get(world, new ClanKey { Id = id });
        }

        protected override string OnSelect(ClanKey key)
        {
            return $"""
                SELECT * FROM `clan`
                WHERE `id` = {key.Id}
                LIMIT 1;
                """;
        }

        protected override string OnUpsert(Clan value)
        {
            var sql = $"""
                INSERT INTO `clan` (
                    `id`,
                    `name`,
                    `title`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Name.Escape()},
                    {value.Title.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `name`=VALUES(`name`),
                    `title`=VALUES(`title`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}