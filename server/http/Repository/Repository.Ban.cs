using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;
using System.Data;

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


        public async Task<Ban> Get(uint world, uint userId)
        {
            await using var conn = _dbContext.GetGlobalConnection(world);
            var value = await conn.QuerySingleOrDefaultAsync<Ban>(OnSelect(new BanKey { User = userId }));
            if (value == null)
                return null;

            if (value.Deleted)
                return null;

            return value;
        }

        protected override string OnSelect(BanKey key)
        {
            return $"""
                SELECT * FROM `ban`
                WHERE `user` = {key.User} AND `deleted` = 0
                LIMIT 1;
                """;
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
                    {value.Deleted.Escape()},
                    {value.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss").Escape()},
                    {value.UpdatedDate.ToString("yyyy-MM-dd HH:mm:ss").Escape()})
                ON DUPLICATE KEY UPDATE
                    `reason` = VALUES(`reason`),
                    `expire_date` = VALUES(`expire_date`),
                    `deleted` = VALUES(`deleted`),
                    `updated_date` = VALUES(`updated_date`);
                """;
            return sql;
        }


        public async Task Delete(uint world, uint userId)
        {
            var ban = await Get(world, userId);
            if (ban == null)
                return;

            ban.Deleted = true;
            // Note: Set method needs to be updated to support world, but for now use direct connection
            await using var conn = _dbContext.GetGlobalConnection(world);
            await conn.ExecuteAsync(OnUpsert(ban));
        }
    }
}
