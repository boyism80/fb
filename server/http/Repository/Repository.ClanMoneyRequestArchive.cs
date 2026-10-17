using Dapper;
using Http.Extension;
using Http.Model;
using Http.Redis;
using Http.Service;
using StackExchange.Redis;

namespace Http.Reepository
{
    public class ClanMoneyRequestArchiveRepository : Repository<ClanMoneyRequestArchive, ClanMoneyRequestArchiveKey>
    {
        private readonly RedisService _redis;
        private readonly ILogger<ClanMoneyRequestArchiveRepository> _logger;

        public ClanMoneyRequestArchiveRepository(DbContext dbContext,
            RedisService redisService,
            ILogger<ClanMoneyRequestArchiveRepository> logger) : base(dbContext)
        {
            _redis = redisService;
            _logger = logger;
        }

        public async Task<ClanMoneyRequestArchive> Get(uint world, uint clan, ulong requestId)
        {
            await DropCacheAsync(world, clan);
            return await base.Get(world, new ClanMoneyRequestArchiveKey
            {
                Clan = clan,
                RequestId = requestId
            });
        }

        public override ClanMoneyRequestArchive Set(uint world, ClanMoneyRequestArchive value)
        {
            _buffer.Enqueue(async () =>
            {
                await DropCacheAsync(world, value.Clan);
                var hash = value.GetHash();
                await using var conn = hash == null
                    ? _dbContext.GetGlobalConnection(world)
                    : _dbContext.GetShardConnection(world, hash.Value);
                await conn.ExecuteAsync(OnUpsert(value));
            });
            return value;
        }

        public override ClanMoneyRequestArchive[] Set(uint world, ClanMoneyRequestArchive[] values)
        {
            if (values == null || values.Length == 0)
                return values;

            _buffer.Enqueue(async () =>
            {
                foreach (var clan in values.Select(x => x.Clan).Distinct())
                {
                    await DropCacheAsync(world, clan);
                }

                foreach (var (conn, items) in _dbContext.GetShardConnections(world, values, value => value.GetHash()))
                {
                    await using (conn)
                    {
                        await conn.ExecuteAsync(OnUpsert(items));
                    }
                }
            });
            return values;
        }

        private async Task DropCacheAsync(uint world, uint clan)
        {
            var redis = _redis.GetShardConnection(world, clan);
            if (redis == null)
                return;

            var key = (RedisKey)$"fb:cache:clan_money_request_archive:{clan}";
            try
            {
                await redis.Connection.KeyDeleteAsync(key);
                await redis.Connection.HashDeleteAsync(Const.ReferenceCountKey, key.ToString());
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to drop clan money archive cache for clan {Clan}", clan);
            }
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
