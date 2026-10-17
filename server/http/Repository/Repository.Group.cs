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

        // Only the create inserts. A later change that reaches the database after the disband finds no row,
        // so it cannot bring the group back.
        protected override string OnUpsert(Group value)
        {
            if (value.Revision <= 1)
            {
                return $"""
                    INSERT IGNORE INTO `group` (
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
                        {value.UpdatedDate.Escape()});
                    """;
            }
            else
            {
                return $"""
                    UPDATE `group` SET
                        `members` = {JsonConvert.SerializeObject(value.Members).Escape()},
                        `revision` = {value.Revision.Escape()},
                        `updated_date` = {value.UpdatedDate.Escape()}
                    WHERE `id` = {value.Id.Escape()} AND `revision` < {value.Revision.Escape()};
                    """;
            }
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
