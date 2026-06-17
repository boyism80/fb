using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class CharacterRealtimeStateRepository : RedisValueRepository<CharacterRealtimeState, CharacterRealtimeStateKey>
    {
        public CharacterRealtimeStateRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<CharacterRealtimeState> Get(uint world, uint uid)
        {
            return await base.Get(world, new CharacterRealtimeStateKey { Uid = uid });
        }

        protected override string OnSelect(CharacterRealtimeStateKey key)
        {
            return $"""
                SELECT * FROM `character_realtime_state`
                WHERE `uid` = {key.Uid} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override CharacterRealtimeStateKey GetKeyFromRow(CharacterRealtimeState row)
        {
            return new CharacterRealtimeStateKey { Uid = row.Uid };
        }

        protected override string OnUpsert(CharacterRealtimeState value)
        {
            var sql = $"""
                INSERT INTO `character_realtime_state` (
                    `uid`,
                    `group`,
                    `clan`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Uid.Escape()},
                    {value.Group.Escape()},
                    {value.Clan.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `group`=VALUES(`group`),
                    `clan`=VALUES(`clan`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}

