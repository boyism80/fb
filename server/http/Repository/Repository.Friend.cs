using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class FriendRepository : RedisHashRepository<Friend, FriendKey>
    {
        public FriendRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<Friend> Get(uint world, uint user, uint friendUid)
        {
            return await base.Get(world, new FriendKey
            {
                User = user,
                FriendUid = friendUid
            });
        }

        public async Task<IEnumerable<Friend>> Get(uint world, uint user)
        {
            return await base.GetAll(world, new FriendKey
            {
                User = user
            });
        }

        protected override string OnSelect(FriendKey key)
        {
            return $"""
                SELECT * FROM `friend` WHERE
                `user` = {key.User} AND
                `friend_uid` = {key.FriendUid} AND
                `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(FriendKey key)
        {
            return $"""
                SELECT * FROM `friend` WHERE
                `user` = {key.User} AND `deleted` = 0;
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<FriendKey> keys)
        {
            return $"SELECT * FROM `friend` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))}) AND `deleted` = 0;";
        }

        protected override FriendKey GetKeyFromRow(Friend row)
        {
            return new FriendKey { User = row.User };
        }

        protected override string OnUpsert(Friend value)
        {
            return $"""
                    INSERT INTO `friend` (
                        `user`,
                        `friend_uid`,
                        `friend_name`,
                        `mutual`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES (
                        {value.User.Escape()},
                        {value.FriendUid.Escape()},
                        {value.FriendName.Escape()},
                        {(value.Mutual ? 1 : 0).Escape()},
                        0,
                        {value.CreatedDate.Escape()},
                        {value.UpdatedDate.Escape()})
                    ON DUPLICATE KEY UPDATE
                        `friend_name`=VALUES(`friend_name`),
                        `mutual`=VALUES(`mutual`),
                        `deleted`=0,
                        `updated_date`=VALUES(`updated_date`);
                    """;
        }

        protected override string OnUpsert(Friend[] values)
        {
            var args = values.Select(friend =>
            {
                return $"""
                        ({friend.User.Escape()},
                         {friend.FriendUid.Escape()},
                         {friend.FriendName.Escape()},
                         {(friend.Mutual ? 1 : 0).Escape()},
                         0,
                         {friend.CreatedDate.Escape()},
                         {friend.UpdatedDate.Escape()})
                        """;
            });

            return $"""
                    INSERT INTO `friend` (
                        `user`,
                        `friend_uid`,
                        `friend_name`,
                        `mutual`,
                        `deleted`,
                        `created_date`,
                        `updated_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `friend_name`=VALUES(`friend_name`),
                        `mutual`=VALUES(`mutual`),
                        `deleted`=0,
                        `updated_date`=VALUES(`updated_date`);
                    """;
        }

        protected override string OnDelete(FriendKey key)
        {
            return $"""
                UPDATE `friend` SET `deleted` = 1, `updated_date` = NOW()
                WHERE `user` = {key.User.Escape()} AND `friend_uid` = {key.FriendUid.Escape()} AND `deleted` = 0;
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<FriendKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var conditions = keys.Select(k =>
                $"(`user` = {k.User.Escape()} AND `friend_uid` = {k.FriendUid.Escape()})");

            return $"""
                UPDATE `friend` SET `deleted` = 1, `updated_date` = NOW()
                WHERE ({string.Join(" OR ", conditions)}) AND `deleted` = 0;
                """;
        }
    }
}
