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
                `id` = {key.Id} AND
                `deleted` = 0
                LIMIT 1;
                """";
        }

        protected override string OnSelectBulk(QuestKey key)
        {
            return $"""
                SELECT * FROM `quest` WHERE
                `user` = {key.User} AND `deleted` = 0;
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<QuestKey> keys)
        {
            return $"SELECT * FROM `quest` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))}) AND `deleted` = 0;";
        }

        protected override QuestKey GetKeyFromRow(Quest row)
        {
            return new QuestKey { User = row.User };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<Quest>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<Quest>>();

            var keys = ownerIds.Distinct().Select(uid => new QuestKey { User = uid }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.User, _ => (IList<Quest>)new List<Quest>());
            foreach (var q in list)
            {
                dict[q.User].Add(q);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<Quest>)kv.Value);
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
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `step`=VALUES(`step`), 
                    `progress`=VALUES(`progress`), 
                    `completed`=VALUES(`completed`),
                    `param`=VALUES(`param`),
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
                         0,
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
                        `updated_date`=VALUES(`updated_date`);
                    """;

            return sql;
        }

        protected override string OnDelete(QuestKey key)
        {
            return $"""
                UPDATE `quest` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `user` = {key.User.Escape()} AND `id` = {key.Id.Escape()} AND `deleted` = 0;
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<QuestKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var conditions = keys.Select(k =>
                $"(`user` = {k.User.Escape()} AND `id` = {k.Id.Escape()})");

            return $"""
                UPDATE `quest` SET `deleted` = 1, `updated_date` = NOW()
                WHERE ({string.Join(" OR ", conditions)}) AND `deleted` = 0;
                """;
        }
    }
}
