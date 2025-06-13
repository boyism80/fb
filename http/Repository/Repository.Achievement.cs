using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class AchievementRepository : RedisHashRepository<Achievement, AchievementKey>
    {
        public AchievementRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {

        }

        public async Task<Achievement> Get(uint uid, uint model)
        {
            return await base.Get(new AchievementKey
            {
                Uid = uid,
                Model = model
            });
        }

        public async Task<IEnumerable<Achievement>> Get(uint uid)
        {
            return await base.GetAll(new AchievementKey
            {
                Uid = uid
            });
        }

        protected override string OnSelect(AchievementKey key)
        {
            var sql = $"""
                SELECT * FROM `achievement` WHERE 
                `uid` = {key.Uid} AND
                `model` = {key.Model}
                LIMIT 1;
                """;

            return sql;
        }

        protected override string OnSelectBulk(AchievementKey key)
        {
            var sql = $"""
                SELECT * FROM `achievement` WHERE
                `uid` = {key.Uid};
                """;
            return sql;
        }

        protected override string OnUpsert(Achievement value)
        {
            var sql = $"""
                    INSERT INTO `achievement` (
                        `uid`,
                        `model`,
                        `text`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES (
                        {value.Uid.Escape()},
                        {value.Model.Escape()},
                        {value.Text.Escape()},
                        {value.Deleted.Escape()},
                        {value.CreatedDate.Escape()},
                        {value.UpdatedDate.Escape()})
                    ON DUPLICATE KEY UPDATE
                        `text`=VALUES(`text`),
                        `deleted`=VALUES(`deleted`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }

        protected override string OnUpsert(Achievement[] values)
        {
            var args = values.Select(achievement =>
            {
                return $"""
                        ({achievement.Uid.Escape()},
                         {achievement.Model.Escape()},
                         {achievement.Text.Escape()},
                         {achievement.Deleted.Escape()},
                         {achievement.CreatedDate.Escape()},
                         {achievement.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO `achievement` (
                        `uid`,
                        `model`,
                        `text`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `text`=VALUES(`text`),
                        `deleted`=VALUES(`deleted`),
                        `created_date`=VALUES(`created_date`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}