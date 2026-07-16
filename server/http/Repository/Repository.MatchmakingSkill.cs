using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class MatchmakingSkillRepository : RedisHashRepository<MatchmakingSkill, MatchmakingSkillKey>
    {
        public MatchmakingSkillRepository(
            DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService)
            : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<MatchmakingSkill> Get(uint world, uint characterId, uint matchType)
        {
            return await base.Get(world, new MatchmakingSkillKey
            {
                CharacterId = characterId,
                MatchType = matchType
            });
        }

        public async Task<IEnumerable<MatchmakingSkill>> Get(uint world, uint characterId)
        {
            return await base.GetAll(world, new MatchmakingSkillKey
            {
                CharacterId = characterId
            });
        }

        public async Task<IReadOnlyDictionary<uint, MatchmakingSkill>> GetByCharacterIds(
            uint world,
            uint matchType,
            IReadOnlyList<uint> characterIds)
        {
            if (characterIds == null || characterIds.Count == 0)
            {
                return new Dictionary<uint, MatchmakingSkill>();
            }

            var skills = new Dictionary<uint, MatchmakingSkill>();
            foreach (var (connection, idBatch) in _dbContext.GetShardConnections(world, characterIds.Distinct()))
            {
                await using (connection)
                {
                    var sql = $"""
                        SELECT * FROM `matchmaking_skill`
                        WHERE `character_id` IN ({string.Join(",", idBatch)})
                          AND `match_type` = {matchType.Escape()};
                        """;

                    var rows = await connection.QueryAsync<MatchmakingSkill>(sql);
                    foreach (var row in rows)
                    {
                        skills[row.CharacterId] = row;
                    }
                }
            }

            return skills;
        }

        protected override string OnSelect(MatchmakingSkillKey key)
        {
            return $"""
                SELECT * FROM `matchmaking_skill`
                WHERE `character_id` = {key.CharacterId.Escape()}
                  AND `match_type` = {key.MatchType.Escape()}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(MatchmakingSkillKey key)
        {
            return $"""
                SELECT * FROM `matchmaking_skill`
                WHERE `character_id` = {key.CharacterId.Escape()};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<MatchmakingSkillKey> keys)
        {
            return $"SELECT * FROM `matchmaking_skill` WHERE `character_id` IN ({string.Join(",", keys.Select(k => k.CharacterId))});";
        }

        protected override MatchmakingSkillKey GetKeyFromRow(MatchmakingSkill row)
        {
            return new MatchmakingSkillKey { CharacterId = row.CharacterId };
        }

        protected override string OnUpsert(MatchmakingSkill value)
        {
            return $"""
                INSERT INTO `matchmaking_skill` (
                    `character_id`,
                    `match_type`,
                    `mu`,
                    `sigma`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.CharacterId.Escape()},
                    {value.MatchType.Escape()},
                    {value.Mu.Escape()},
                    {value.Sigma.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `mu` = VALUES(`mu`),
                    `sigma` = VALUES(`sigma`),
                    `updated_date` = VALUES(`updated_date`);
                """;
        }

        protected override string OnUpsert(MatchmakingSkill[] values)
        {
            var args = values.Select(value => $"""
                ({value.CharacterId.Escape()},
                 {value.MatchType.Escape()},
                 {value.Mu.Escape()},
                 {value.Sigma.Escape()},
                 {value.CreatedDate.Escape()},
                 {value.UpdatedDate.Escape()})
                """);

            return $"""
                INSERT INTO `matchmaking_skill` (
                    `character_id`,
                    `match_type`,
                    `mu`,
                    `sigma`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `mu` = VALUES(`mu`),
                    `sigma` = VALUES(`sigma`),
                    `updated_date` = VALUES(`updated_date`);
                """;
        }
    }
}
