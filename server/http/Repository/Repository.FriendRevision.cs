using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class FriendRevisionRepository : RedisValueRepository<FriendRevision, FriendRevisionKey>
    {
        public FriendRevisionRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<FriendRevision> Get(uint world, uint user)
        {
            return await base.Get(world, new FriendRevisionKey { User = user });
        }

        protected override string OnSelect(FriendRevisionKey key)
        {
            return $"""
                SELECT * FROM `friend_revision`
                WHERE `user` = {key.User}
                LIMIT 1;
                """;
        }

        protected override FriendRevisionKey GetKeyFromRow(FriendRevision row)
        {
            return new FriendRevisionKey { User = row.User };
        }

        protected override string OnUpsert(FriendRevision value)
        {
            return $"""
                INSERT INTO `friend_revision` (
                    `user`,
                    `revision`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.Revision.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `revision`=VALUES(`revision`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}
