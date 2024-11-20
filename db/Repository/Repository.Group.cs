using Db.Extension;
using Db.Model;
using Db.Service;
using http.Service;
using Newtonsoft.Json;

namespace Db.Reepository
{
    public class GroupRepository : RedisValueRepository<Group, GroupKey>
    {
        public GroupRepository(DbContext dbContext,
            RedisService redisService,
            DbExecuteService dbExecuteService) : base(dbContext, redisService, dbExecuteService)
        {
        }

        public async Task<Group> Get(uint master)
        {
            return await Get(new GroupKey { Master = master });
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