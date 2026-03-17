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

        public async Task<Achievement> Get(uint world, uint uid, uint id)
        {
            return await base.Get(world, new AchievementKey
            {
                Uid = uid,
                Id = id
            });
        }

        public async Task<IEnumerable<Achievement>> Get(uint world, uint uid)
        {
            return await base.GetAll(world, new AchievementKey
            {
                Uid = uid
            });
        }

        protected override string OnSelect(AchievementKey key)
        {
            var sql = $"""
                SELECT * FROM `achievement` WHERE 
                `uid` = {key.Uid} AND
                `id` = {key.Id}
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
                        `id`,
                        `text`,
                        `icon`,
                        `color`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES (
                        {value.Uid.Escape()},
                        {value.Id.Escape()},
                        {value.Text.Escape()},
                        {value.Icon.Escape()},
                        {value.Color.Escape()},
                        {value.Deleted.Escape()},
                        {value.CreatedDate.Escape()},
                        {value.UpdatedDate.Escape()})
                    ON DUPLICATE KEY UPDATE
                        `text`=VALUES(`text`),
                        `icon`=VALUES(`icon`),
                        `color`=VALUES(`color`),
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
                         {achievement.Id.Escape()},
                         {achievement.Text.Escape()},
                         {achievement.Icon.Escape()},
                         {achievement.Color.Escape()},
                         {achievement.Deleted.Escape()},
                         {achievement.CreatedDate.Escape()},
                         {achievement.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO `achievement` (
                        `uid`,
                        `id`,
                        `text`,
                        `icon`,
                        `color`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `text`=VALUES(`text`),
                        `icon`=VALUES(`icon`),
                        `color`=VALUES(`color`),
                        `deleted`=VALUES(`deleted`),
                        `created_date`=VALUES(`created_date`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }
    }
}