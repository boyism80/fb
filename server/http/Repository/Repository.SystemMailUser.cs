using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class SystemMailUserRepository : RedisHashRepository<SystemMailUser, SystemMailUserKey>
    {
        public SystemMailUserRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public async Task<SystemMailUser> Get(uint world, uint user, uint mailId)
        {
            return await base.Get(world, new SystemMailUserKey
            {
                User = user,
                MailId = mailId
            });
        }

        public async Task<IEnumerable<SystemMailUser>> Get(uint world, uint user)
        {
            return await base.GetAll(world, new SystemMailUserKey
            {
                User = user,
                MailId = 0
            });
        }

        protected override string OnSelect(SystemMailUserKey key)
        {
            return $"""
                SELECT * FROM `system_mail_user` WHERE 
                `user` = {key.User} AND
                `mail_id` = {key.MailId}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(SystemMailUserKey key)
        {
            return $"""
                SELECT * FROM `system_mail_user` WHERE
                `user` = {key.User};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<SystemMailUserKey> keys)
        {
            return $"SELECT * FROM `system_mail_user` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))});";
        }

        protected override SystemMailUserKey GetKeyFromRow(SystemMailUser row)
        {
            return new SystemMailUserKey { User = row.User, MailId = row.MailId };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<SystemMailUser>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<SystemMailUser>>();

            var keys = ownerIds.Distinct().Select(uid => new SystemMailUserKey { User = uid, MailId = 0 }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.User, _ => (IList<SystemMailUser>)new List<SystemMailUser>());
            foreach (var m in list)
            {
                dict[m.User].Add(m);
            }
            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<SystemMailUser>)kv.Value);
        }

        protected override string OnUpsert(SystemMailUser value)
        {
            var sql = $"""
                INSERT INTO `system_mail_user` (
                    `mail_id`,
                    `user`,
                    `read`,
                    `deleted`,
                    `created_date`)
                VALUES (
                    {value.MailId.Escape()},
                    {value.User.Escape()},
                    {value.Read.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `read`=VALUES(`read`), 
                    `deleted`=VALUES(`deleted`);
                """;

            return sql;
        }

        protected override string OnUpsert(SystemMailUser[] values)
        {
            var args = values.Select(smu =>
            {
                return $"""
                        ({smu.MailId.Escape()},
                         {smu.User.Escape()},
                         {smu.Read.Escape()},
                         {smu.Deleted.Escape()},
                         {smu.CreatedDate.Escape()})
                        """;
            });

            var sql = $"""
                    INSERT INTO system_mail_user (
                        `mail_id`,
                        `user`,
                        `read`,
                        `deleted`,
                        `created_date`)
                    VALUES {string.Join(',', args)}
                    ON DUPLICATE KEY UPDATE
                        `read`=VALUES(`read`),
                        `deleted`=VALUES(`deleted`);
                    """;

            return sql;
        }
    }
}
