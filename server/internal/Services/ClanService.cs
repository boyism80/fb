using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Medallion.Threading.Redis;
using Protocol = fb.protocol.@internal;
using Response = fb.protocol.@internal.response;

namespace Internal.Services
{
    public class ClanService
    {
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly ILogger<ClanService> _logger;

        public ClanService(DbContext dbContext,
            RabbitMqService rabbitMqService,
            RedisDistributedLockService distributedLock,
            ILogger<ClanService> logger)
        {
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _distributedLock = distributedLock;
            _logger = logger;
        }

        public sealed class ClanLocks : IAsyncDisposable
        {
            private readonly List<RedisDistributedLockHandle> _locks = new List<RedisDistributedLockHandle>();

            public void Add(RedisDistributedLockHandle handle)
            {
                _locks.Add(handle);
            }

            public async ValueTask DisposeAsync()
            {
                for (var i = _locks.Count - 1; i >= 0; i--)
                {
                    await _locks[i].DisposeAsync();
                }
            }
        }

        public void Touch(uint world, params Clan[] clans)
        {
            var at = DateTime.UtcNow;
            foreach (var clan in clans)
            {
                clan.UpdatedDate = at;
                clan.Revision++;
                _dbContext.Clan.Set(world, clan);
            }
        }

        // Every path that touches more than one clan locks them in id order so two such paths cannot deadlock.
        public async Task<ClanLocks> LockClans(uint world, IEnumerable<uint> ids)
        {
            var locks = new ClanLocks();
            try
            {
                foreach (var id in ids.Distinct().OrderBy(x => x))
                {
                    locks.Add(await _distributedLock.Lock(world, Clan.DistributedLockKey(id)));
                }
                return locks;
            }
            catch
            {
                await locks.DisposeAsync();
                throw;
            }
        }

        public async Task<List<uint>> GetCounterparts(uint world, uint id)
        {
            var ids = (await _dbContext.ClanEnemy.Get(world, id)).Select(x => x.EnemyClan).ToList();
            var alliance = await _dbContext.ClanAlliance.Get(world, id);
            if (alliance != null)
            {
                ids.Add(alliance.AlliedClan);
            }
            return ids;
        }

        public async Task<Response.DestroyClan> Remove(uint world, Clan clan, IReadOnlyCollection<uint> counterpartIds, uint host, Protocol.CharacterRef actor)
        {
            var castleLocks = new List<RedisDistributedLockHandle>();
            try
            {
                var alliance = await _dbContext.ClanAlliance.Get(world, clan.Id);
                await using var _ = alliance != null ? await _distributedLock.Lock(world, ClanAlliance.DistributedLockKey(alliance.AlliedClan)) : null;
                var unallied = default(Clan);
                if (alliance != null)
                {
                    if (counterpartIds.Contains(alliance.AlliedClan) == false)
                    {
                        throw new LogicException(ErrorCode.Unhandled);
                    }

                    _dbContext.ClanAlliance.Delete(world, clan.Id);
                    _dbContext.ClanAlliance.Delete(world, alliance.AlliedClan);
                    unallied = await _dbContext.Clan.Get(world, alliance.AlliedClan);
                    if (unallied != null)
                    {
                        Touch(world, unallied);
                    }
                }

                var enemies = (await _dbContext.ClanEnemy.Get(world, clan.Id)).ToList();
                var unenemies = new List<Clan>();
                foreach (var enemy in enemies)
                {
                    if (counterpartIds.Contains(enemy.EnemyClan) == false)
                    {
                        throw new LogicException(ErrorCode.Unhandled);
                    }

                    _dbContext.ClanEnemy.Delete(world, enemy);
                    var reverse = await _dbContext.ClanEnemy.Get(world, enemy.EnemyClan, clan.Id);
                    if (reverse != null)
                    {
                        _dbContext.ClanEnemy.Delete(world, reverse);
                    }

                    var other = await _dbContext.Clan.Get(world, enemy.EnemyClan);
                    if (other != null)
                    {
                        Touch(world, other);
                        unenemies.Add(other);
                    }
                }

                var clearedCastles = new List<byte>();
                for (byte divineBeast = 0; divineBeast <= 3; divineBeast++)
                {
                    castleLocks.Add(await _distributedLock.Lock(world, Castle.DistributedLockKey(divineBeast)));
                    var castleRecord = await _dbContext.Castle.Get(world, divineBeast);
                    if (castleRecord?.OwnerClan == clan.Id)
                    {
                        castleRecord.OwnerClan = null;
                        _dbContext.Castle.Set(world, castleRecord);
                        clearedCastles.Add(divineBeast);
                    }
                }

                var archived = clan.MoneyRequests
                    .Select(x => new ClanMoneyRequestArchive { Clan = clan.Id, RequestId = x.Id, Applied = x.Applied })
                    .ToArray();
                if (archived.Length > 0)
                {
                    _dbContext.ClanMoneyRequestArchive.Set(world, archived);
                    await _dbContext.ClanMoneyRequestArchive.SaveChangesAsync();
                }

                await _dbContext.SaveChangesAsync();

                _dbContext.Clan.Delete(world, clan.Id);
                await _dbContext.Clan.SaveChangesAsync();

                var response = new Response.DestroyClan
                {
                    Host = host,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Actor = actor,
                    Revision = clan.Revision + 1,
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.NotifySavedAsync(response, "clan", world);

                // The destroying game server gets no HTTP response for these clans, so no host may skip them.
                if (unallied != null)
                {
                    await _rabbitMqService.NotifySavedAsync(new Response.UpdatedClan
                    {
                        Host = uint.MaxValue,
                        Action = Protocol.ClanActionType.Unally,
                        ClanId = unallied.Id,
                        ClanName = unallied.Name,
                        Revision = unallied.Revision,
                        Actor = actor,
                        RelatedClanId = clan.Id,
                        Error = (uint)ErrorCode.None
                    }, "clan", world);
                }
                foreach (var other in unenemies)
                {
                    await _rabbitMqService.NotifySavedAsync(new Response.UpdatedClan
                    {
                        Host = uint.MaxValue,
                        Action = Protocol.ClanActionType.Unenemy,
                        ClanId = other.Id,
                        ClanName = other.Name,
                        Revision = other.Revision,
                        Actor = actor,
                        RelatedClanId = clan.Id,
                        Error = (uint)ErrorCode.None
                    }, "clan", world);
                }

                foreach (var divineBeast in clearedCastles)
                {
                    await _rabbitMqService.NotifySavedAsync(new Response.UpdatedCastle
                    {
                        Host = uint.MaxValue,
                        DivineBeast = divineBeast,
                        OwnerClanId = null,
                        Error = (uint)ErrorCode.None
                    }, AmqpRoute.Exchange, AmqpRoute.Home("castle", world));
                }

                try
                {
                    await using var db = _dbContext.GetUnifiedConnection();
                    await db.ExecuteAsync("USP_CLAN_NAME_DELETE", new
                    {
                        id = clan.Id
                    }, commandType: System.Data.CommandType.StoredProcedure);
                }
                catch (Exception e)
                {
                    _logger.LogWarning(e, "Failed to remove the name of destroyed clan {Id} in world {World}", clan.Id, world);
                }
                return response;
            }
            finally
            {
                foreach (var handle in castleLocks)
                {
                    await handle.DisposeAsync();
                }
            }
        }
    }
}
