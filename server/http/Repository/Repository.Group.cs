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
                WHERE `master` = {key.Master}
                LIMIT 1;
                """;
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
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `members`=VALUES(`members`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}