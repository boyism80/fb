using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class QuestRepository : RedisHashRepository<Quest, QuestKey>
    {
        public QuestRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<Quest> Get(uint world, uint user, uint id)
        {
            return await base.Get(world, new QuestKey
            {
                User = user,
                Id = id
            });
        }

        public async Task<IEnumerable<Quest>> Get(uint world, uint user)
        {
            return await base.GetAll(world, new QuestKey
            {
                User = user
            });
        }

        protected override string OnSelect(QuestKey key)
        {
            return $""""
                SELECT * FROM `quest` WHERE 
                `user` = {key.User} AND
                `id` = {key.Id}
                LIMIT 1;
                """";
        }

        protected override string OnSelectBulk(QuestKey key)
        {
            return $"""
                SELECT * FROM `quest` WHERE
                `user` = {key.User};
                """;
        }

        protected override string OnUpsert(Quest value)
        {
            var sql = $"""
                INSERT INTO `quest` (
                    `user`,
                    `id`,
                    `step`,
                    `progress`,
                    `completed`,
                    `param`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.Id.Escape()},
                    {value.Step.Escape()},
                    {value.Progress.Escape()},
                    {value.Completed.Escape()},
                    {value.Param.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `step`=VALUES(`step`), 
                    `progress`=VALUES(`progress`), 
                    `completed`=VALUES(`completed`),
                    `param`=VALUES(`param`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        protected override string OnUpsert(Quest[] values)
        {
            var args = values.Select(item =>
            {
                return $"""
                        ({item.User.Escape()},
                         {item.Id.Escape()},
                         {item.Step.Escape()},
                         {item.Progress.Escape()},
                         {item.Completed.Escape()},
                         {item.Param.Escape()},
                         {item.Deleted.Escape()},
                         {item.CreatedDate.Escape()},
                         {item.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO quest (
                        `user`,
                        `id`,
                        `step`,
                        `progress`,
                        `completed`,
                        `param`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `step`=VALUES(`step`),
                        `progress`=VALUES(`progress`),
                        `completed`=VALUES(`completed`),
                        `param`=VALUES(`param`),
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}