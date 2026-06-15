using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class BanRepository : RedisValueRepository<Ban, BanKey>
    {
        public BanRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<Ban> Get(uint world, uint userId) =>
            base.Get(world, new BanKey { User = userId });

        protected override string OnSelect(BanKey key)
        {
            return $"""
                SELECT * FROM `ban`
                WHERE `user` = {key.User} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override BanKey GetKeyFromRow(Ban row)
        {
            return new BanKey { User = row.User };
        }

        protected override string OnUpsert(Ban value)
        {
            var sql = $"""
                INSERT INTO `ban` (
                    `user`,
                    `reason`,
                    `expire_date`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.Reason.Escape()},
                    {value.ExpireDate.Escape()},
                    0,
                    {value.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss").Escape()},
                    {value.UpdatedDate.ToString("yyyy-MM-dd HH:mm:ss").Escape()})
                ON DUPLICATE KEY UPDATE
                    `reason` = VALUES(`reason`),
                    `expire_date` = VALUES(`expire_date`),
                    `deleted` = 0,
                    `updated_date` = VALUES(`updated_date`);
                """;
            return sql;
        }

        protected override string OnDelete(BanKey key)
        {
            return $"""
                UPDATE `ban` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `user` = {key.User.Escape()} AND `deleted` = 0;
                """;
        }

        public void Delete(uint world, uint userId)
        {
            base.Delete(world, new BanKey { User = userId });
        }
    }
}
