using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class TraceRepository : RedisHashRepository<Trace, TraceKey>
    {
        public TraceRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {

        }

        public async Task<Trace> Get(uint uid, uint model)
        {
            return await base.Get(new TraceKey
            {
                Uid = uid,
                Model = model
            });
        }

        public async Task<IEnumerable<Trace>> Get(uint uid)
        {
            return await base.GetAll(new TraceKey
            {
                Uid = uid
            });
        }

        protected override string OnSelect(TraceKey key)
        {
            var sql = $"""
                SELECT * FROM `trace` WHERE 
                `uid` = {key.Uid} AND
                `model` = {key.Model}
                LIMIT 1;
                """;

            return sql;
        }

        protected override string OnSelectBulk(TraceKey key)
        {
            var sql = $"""
                SELECT * FROM `trace` WHERE
                `uid` = {key.Uid};
                """;
            return sql;
        }

        protected override string OnUpsert(Trace value)
        {
            var sql = $"""
                    INSERT INTO `trace` (
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

        protected override string OnUpsert(Trace[] values)
        {
            var args = values.Select(trace =>
            {
                return $"""
                        ({trace.Uid.Escape()},
                         {trace.Model.Escape()},
                         {trace.Text.Escape()},
                         {trace.Deleted.Escape()},
                         {trace.CreatedDate.Escape()},
                         {trace.UpdatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO `trace` (
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