using Dapper;
using Http.Model;
using Http.Service;
using Protocol = fb.protocol.@internal;
using Response = fb.protocol.@internal.response;

namespace Internal.Services
{
    public class ClanCleanupBackgroundService : BackgroundService
    {
        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(1);
        private const int RowLimit = 500;

        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly RabbitMqService _rabbitMqService;
        private readonly ILogger<ClanCleanupBackgroundService> _logger;

        public ClanCleanupBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            RabbitMqService rabbitMqService,
            ILogger<ClanCleanupBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _redisService = redisService;
            _rabbitMqService = rabbitMqService;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                foreach (var world in _redisService.GetConfiguredWorlds())
                {
                    if (stoppingToken.IsCancellationRequested)
                    {
                        break;
                    }

                    try
                    {
                        await RemoveEmptyClansAsync(world);
                        await RemoveOrphanNamesAsync(world);
                    }
                    catch (Exception e)
                    {
                        _logger.LogWarning(e, "Failed to clean up clans for world {World}", world);
                    }
                }

                await Task.Delay(ProcessingInterval, stoppingToken);
            }
        }

        private async Task RemoveEmptyClansAsync(uint world)
        {
            var ids = new List<uint>();
            using (var scope = _scopeFactory.CreateScope())
            {
                var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();
                for (var db = 0; db < dbContext.GetShardDbSize(world); db++)
                {
                    await using var conn = dbContext.GetDataConnection(world, db);
                    ids.AddRange(await conn.QueryAsync<uint>(
                        """
                        SELECT c.`id` FROM `clan` c
                        WHERE c.`deleted` = 0 AND NOT EXISTS (SELECT 1 FROM `clan_member` m WHERE m.`clan` = c.`id` AND m.`deleted` = 0)
                        LIMIT @limit
                        """,
                        new { limit = RowLimit }));
                }
            }

            foreach (var id in ids)
            {
                try
                {
                    using var scope = _scopeFactory.CreateScope();
                    var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();
                    var clans = scope.ServiceProvider.GetRequiredService<ClanService>();

                    var counterpartIds = await clans.GetCounterparts(world, id);
                    await using var _ = await clans.LockClans(world, counterpartIds.Append(id));

                    var clan = await dbContext.Clan.Get(world, id);
                    if (clan == null || (await dbContext.ClanMember.Get(world, id)).Any())
                    {
                        continue;
                    }

                    if (clan.Money > 0)
                    {
                        _logger.LogWarning("Clan {Id} in world {World} has no member but keeps {Money} money", id, world, clan.Money);
                        continue;
                    }

                    await clans.Remove(world, clan, counterpartIds, uint.MaxValue, new Protocol.CharacterRef());
                    _logger.LogInformation("Removed clan {Id} without members in world {World}", id, world);
                }
                catch (Exception e)
                {
                    _logger.LogWarning(e, "Failed to remove clan {Id} without members in world {World}", id, world);
                }
            }
        }

        private async Task RemoveOrphanNamesAsync(uint world)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();
            var distributedLock = scope.ServiceProvider.GetRequiredService<RedisDistributedLockService>();

            var names = new Dictionary<uint, string>();
            await using (var unified = dbContext.GetUnifiedConnection())
            {
                foreach (var row in await unified.QueryAsync<(uint Id, string Name)>(
                    "SELECT `id`, `name` FROM `clan_name` WHERE `world` = @world",
                    new { world }))
                {
                    names[row.Id] = row.Name;
                }
            }

            var named = names.Keys.ToList();

            var existing = new HashSet<uint>();
            foreach (var (conn, idList) in dbContext.GetShardConnections(world, named))
            {
                await using (conn)
                {
                    existing.UnionWith(await conn.QueryAsync<uint>(
                        "SELECT `id` FROM `clan` WHERE `deleted` = 0 AND `id` IN @ids",
                        new { ids = idList }));
                }
            }

            foreach (var id in named.Where(x => existing.Contains(x) == false))
            {
                try
                {
                    await using var _ = await distributedLock.Lock(world, Clan.DistributedLockKey(id));
                    if (await dbContext.Clan.Get(world, id) != null)
                    {
                        continue;
                    }

                    await _rabbitMqService.NotifySavedAsync(new Response.DestroyClan
                    {
                        Host = uint.MaxValue,
                        ClanId = id,
                        ClanName = names[id],
                        Actor = new Protocol.CharacterRef { Uid = 0, Name = string.Empty },
                        Revision = ulong.MaxValue,
                        Error = (uint)Fb.Model.EnumValue.ErrorCode.None
                    }, "clan", world);

                    await using var unified = dbContext.GetUnifiedConnection();
                    await unified.ExecuteAsync("USP_CLAN_NAME_DELETE", new
                    {
                        id
                    }, commandType: System.Data.CommandType.StoredProcedure);
                    _logger.LogInformation("Removed clan name {Id} without a clan in world {World}", id, world);
                }
                catch (Exception e)
                {
                    _logger.LogWarning(e, "Failed to remove clan name {Id} without a clan in world {World}", id, world);
                }
            }
        }
    }
}
