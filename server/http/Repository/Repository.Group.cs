using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;

namespace Http.Reepository
{
    public class GroupRepository : RedisValueRepository<Group, GroupKey>
    {
        public GroupRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<Group> Get(uint world, uint master)
        {
            return await base.Get(world, new GroupKey { Master = master });
        }

        protected override string OnSelect(GroupKey key)
        {
            return $"""
                SELECT * FROM `group`
                WHERE `master` = {key.Master} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override GroupKey GetKeyFromRow(Group row)
        {
            return new GroupKey { Master = row.Master };
        }

        protected override string OnUpsert(Group value)
        {
            var sql = $"""
                INSERT INTO `group` (
                    `master`,
                    `members`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Master.Escape()},
                    {JsonConvert.SerializeObject(value.Members).Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `members`=VALUES(`members`),
                    `deleted` = 0,
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        protected override string OnDelete(GroupKey key)
        {
            return $"""
                UPDATE `group` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `master` = {key.Master.Escape()} AND `deleted` = 0;
                """;
        }

        public void Delete(uint world, uint master)
        {
            base.Delete(world, new GroupKey { Master = master });
        }
    }
}
