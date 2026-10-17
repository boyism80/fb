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

        public async Task<Group> Get(uint world, ulong id)
        {
            return await base.Get(world, new GroupKey { Id = id });
        }

        protected override string OnSelect(GroupKey key)
        {
            return $"""
                SELECT * FROM `group`
                WHERE `id` = {key.Id} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override GroupKey GetKeyFromRow(Group row)
        {
            return new GroupKey { Id = row.Id };
        }

        protected override string OnUpsert(Group value)
        {
            var sql = $"""
                INSERT INTO `group` (
                    `id`,
                    `master`,
                    `members`,
                    `revision`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Master.Escape()},
                    {JsonConvert.SerializeObject(value.Members).Escape()},
                    {value.Revision.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `members`=VALUES(`members`),
                    `revision`=VALUES(`revision`),
                    `deleted` = 0,
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        protected override string OnDelete(GroupKey key)
        {
            return $"""
                UPDATE `group` SET `deleted` = 1, `updated_date` = UTC_TIMESTAMP(3)
                WHERE `id` = {key.Id.Escape()} AND `deleted` = 0;
                """;
        }

        public void Delete(uint world, ulong id)
        {
            base.Delete(world, new GroupKey { Id = id });
        }
    }
}
