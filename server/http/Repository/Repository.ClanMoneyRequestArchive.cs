using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class ClanMoneyRequestArchiveRepository : RedisHashRepository<ClanMoneyRequestArchive, ClanMoneyRequestArchiveKey>
    {
        public ClanMoneyRequestArchiveRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<ClanMoneyRequestArchive> Get(uint world, uint clan, ulong requestId)
        {
            return base.Get(world, new ClanMoneyRequestArchiveKey
            {
                Clan = clan,
                RequestId = requestId
            });
        }

        protected override string OnSelect(ClanMoneyRequestArchiveKey key)
        {
            return $"""
                SELECT * FROM `clan_money_request_archive`
                WHERE `clan` = {key.Clan} AND `request_id` = {key.RequestId.Escape()}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(ClanMoneyRequestArchiveKey key)
        {
            return $"""
                SELECT * FROM `clan_money_request_archive`
                WHERE `clan` = {key.Clan};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<ClanMoneyRequestArchiveKey> keys)
        {
            return $"SELECT * FROM `clan_money_request_archive` WHERE `clan` IN ({string.Join(",", keys.Select(k => k.Clan))});";
        }

        protected override ClanMoneyRequestArchiveKey GetKeyFromRow(ClanMoneyRequestArchive row)
        {
            return new ClanMoneyRequestArchiveKey { Clan = row.Clan, RequestId = row.RequestId };
        }

        protected override string OnDelete(ClanMoneyRequestArchiveKey key)
        {
            return $"""
                DELETE FROM `clan_money_request_archive`
                WHERE `clan` = {key.Clan.Escape()} AND `request_id` = {key.RequestId.Escape()};
                """;
        }

        protected override string OnDeleteMany(IReadOnlyList<ClanMoneyRequestArchiveKey> keys)
        {
            if (keys.Count == 0)
                return string.Empty;

            var clan = keys[0].Clan;
            return $"""
                DELETE FROM `clan_money_request_archive`
                WHERE `clan` = {clan.Escape()} AND `request_id` IN ({string.Join(",", keys.Select(k => k.RequestId.Escape()))});
                """;
        }

        protected override string OnUpsert(ClanMoneyRequestArchive value)
        {
            return OnUpsert(new[] { value });
        }

        protected override string OnUpsert(ClanMoneyRequestArchive[] values)
        {
            if (values.Length == 0)
                return string.Empty;

            var args = values.Select(value => $"""
                ({value.Clan.Escape()},
                 {value.RequestId.Escape()},
                 {value.Applied.Escape()},
                 {value.CreatedDate.Escape()},
                 {value.UpdatedDate.Escape()})
                """);

            return $"""
                INSERT INTO `clan_money_request_archive` (
                    `clan`,
                    `request_id`,
                    `applied`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `applied`=VALUES(`applied`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}
