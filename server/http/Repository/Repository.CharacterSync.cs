using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class CharacterSyncRepository : RedisValueRepository<CharacterSync, CharacterSyncKey>
    {
        public CharacterSyncRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<CharacterSync> Get(uint world, uint uid)
        {
            return await base.Get(world, new CharacterSyncKey { Uid = uid });
        }

        protected override string OnSelect(CharacterSyncKey key)
        {
            return $"""
                SELECT * FROM `character_sync`
                WHERE `uid` = {key.Uid}
                LIMIT 1;
                """;
        }

        protected override string OnUpsert(CharacterSync value)
        {
            var sql = $"""
                INSERT INTO `character_sync` (
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
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `group`=VALUES(`group`),
                    `clan`=VALUES(`clan`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}