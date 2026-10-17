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
                WHERE `id` = {key.Id}
                LIMIT 1;
                """;
        }

        protected override GroupKey GetKeyFromRow(Group row)
        {
            return new GroupKey { Id = row.Id };
        }

        protected override string OnUpsert(Group value)
        {
            // `revision` must be assigned last: each assignment sees the columns already updated before it.
            var sql = $"""
                INSERT INTO `group` (
                    `id`,
                    `master`,
                    `members`,
                    `revision`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Master.Escape()},
                    {JsonConvert.SerializeObject(value.Members).Escape()},
                    {value.Revision.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `members` = IF(VALUES(`revision`) > `revision`, VALUES(`members`), `members`),
                    `updated_date` = IF(VALUES(`revision`) > `revision`, VALUES(`updated_date`), `updated_date`),
                    `revision` = GREATEST(`revision`, VALUES(`revision`));
                """;

            return sql;
        }

        protected override string OnDelete(GroupKey key)
        {
            return $"""
                DELETE FROM `group`
                WHERE `id` = {key.Id.Escape()};
                """;
        }

        public void Delete(uint world, ulong id)
        {
            base.Delete(world, new GroupKey { Id = id });
        }
    }
}
