using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class MarriageRepository : RedisValueRepository<Marriage, MarriageKey>
    {
        public MarriageRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<Marriage> Get(uint world, uint characterId)
        {
            return await base.Get(world, new MarriageKey { CharacterId = characterId });
        }

        public async Task<IReadOnlyDictionary<uint, DateTime>> GetCreatedDates(uint world, IReadOnlyList<uint> characterIds)
        {
            if (characterIds == null || characterIds.Count == 0)
                return new Dictionary<uint, DateTime>();

            var created = new Dictionary<uint, DateTime>();
            foreach (var (connection, idBatch) in _dbContext.GetShardConnections(world, characterIds.Distinct()))
            {
                await using (connection)
                {
                    var rows = await connection.QueryAsync<Marriage>($"""
                        SELECT
                            character_id AS CharacterId,
                            created_date AS CreatedDate
                        FROM `marriage`
                        WHERE `character_id` IN ({string.Join(",", idBatch)});
                        """);
                    foreach (var row in rows)
                        created[row.CharacterId] = row.CreatedDate;
                }
            }

            return created;
        }

        protected override string OnSelect(MarriageKey key)
        {
            return $"""
                SELECT
                    character_id AS CharacterId,
                    spouse_id AS SpouseId,
                    COALESCE(remarriage_after, UTC_TIMESTAMP()) AS RemarriageAfter,
                    divorce_count AS DivorceCount,
                    created_date AS CreatedDate,
                    updated_date AS UpdatedDate
                FROM `marriage`
                WHERE `character_id` = {key.CharacterId}
                LIMIT 1;
                """;
        }

        protected override MarriageKey GetKeyFromRow(Marriage row)
        {
            return new MarriageKey { CharacterId = row.CharacterId };
        }

        protected override string OnUpsert(Marriage value)
        {
            var spouseIdSql = value.SpouseId.HasValue ? value.SpouseId.Value.Escape() : "NULL";
            var remarriageAfterSql = value.RemarriageAfter.ToString("yyyy-MM-dd HH:mm:ss").Escape();

            var sql = $"""
                INSERT INTO `marriage` (
                    `character_id`,
                    `spouse_id`,
                    `remarriage_after`,
                    `divorce_count`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.CharacterId.Escape()},
                    {spouseIdSql},
                    {remarriageAfterSql},
                    {value.DivorceCount.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `spouse_id` = VALUES(`spouse_id`),
                    `remarriage_after` = VALUES(`remarriage_after`),
                    `divorce_count` = VALUES(`divorce_count`),
                    `created_date` = COALESCE(`created_date`, VALUES(`created_date`)),
                    `updated_date` = VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}
