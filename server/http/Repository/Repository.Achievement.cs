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
                `id` = {key.Id} AND
                `deleted` = 0
                LIMIT 1;
                """;

            return sql;
        }

        protected override string OnSelectBulk(AchievementKey key)
        {
            var sql = $"""
                SELECT * FROM `achievement` WHERE
                `uid` = {key.Uid} AND `deleted` = 0;
                """;
            return sql;
        }

        protected override string OnSelectMany(IReadOnlyList<AchievementKey> keys)
        {
            return $"SELECT * FROM `achievement` WHERE `uid` IN ({string.Join(",", keys.Select(k => k.Uid))}) AND `deleted` = 0;";
        }

        protected override AchievementKey GetKeyFromRow(Achievement row)
        {
            return new AchievementKey { Uid = row.Uid };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<Achievement>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<Achievement>>();

            var keys = ownerIds.Distinct().Select(uid => new AchievementKey { Uid = uid }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.Uid, _ => (IList<Achievement>)new List<Achievement>());
            foreach (var a in list)
            {
                dict[a.Uid].Add(a);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<Achievement>)kv.Value);
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
                        0,
                        {value.CreatedDate.Escape()},
                        {value.UpdatedDate.Escape()})
                    ON DUPLICATE KEY UPDATE
                        `text`=VALUES(`text`),
                        `icon`=VALUES(`icon`),
                        `color`=VALUES(`color`),
                        `deleted`=0,
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
                         0,
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
                        `deleted`=0,
                        `created_date`=VALUES(`created_date`),
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }

        protected override string OnDelete(AchievementKey key)
        {
            return $"""
                UPDATE `achievement` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `uid` = {key.Uid.Escape()} AND `id` = {key.Id.Escape()} AND `deleted` = 0;
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<AchievementKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var conditions = keys.Select(k =>
                $"(`uid` = {k.Uid.Escape()} AND `id` = {k.Id.Escape()})");

            return $"""
                UPDATE `achievement` SET `deleted` = 1, `updated_date` = NOW()
                WHERE ({string.Join(" OR ", conditions)}) AND `deleted` = 0;
                """;
        }
    }
}
