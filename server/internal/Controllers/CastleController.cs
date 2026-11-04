using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Protocol = fb.protocol.@internal;
using Request = fb.protocol.@internal.request;
using Response = fb.protocol.@internal.response;

namespace Internal.Controllers
{
    // Known limits of this design (internal owns the siege state, the game hosts only request and report):
    // - The siege length follows UtcNow here, not the adjusted game clock, so /시간가속 does not shorten a siege.
    // - A round whose heart host never reports stays unended but inactive after SiegeGrace, until the next start.
    // - A report that arrives after SiegeGrace ends the round without changing the owner.
    // - Clan destruction also writes the owner (ClanService.Remove); that keeps the clan reference valid and is intended.
    [ApiController]
    [Route("castle")]
    public class CastleController : ControllerBase
    {
        private static readonly byte[] AllDivineBeasts = { 0, 1, 2, 3 };
        private const int MinSiegeSeconds = 1800;
        private const int MaxSiegeSeconds = 3600;

        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly RedisDistributedLockService _distributedLock;

        public CastleController(DbContext dbContext,
            RabbitMqService rabbitMqService,
            RedisDistributedLockService distributedLock)
        {
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _distributedLock = distributedLock;
        }

        private async Task<Castle> Load(uint world, byte divineBeast)
        {
            return await _dbContext.Castle.Get(world, divineBeast) ?? new Castle { DivineBeast = divineBeast };
        }

        [HttpGet("{world}")]
        public async Task<Response.CastleList> Get(uint world)
        {
            try
            {
                var now = DateTime.UtcNow;
                var castles = new List<Protocol.Castle>();
                foreach (var divineBeast in AllDivineBeasts)
                {
                    var castle = await _dbContext.Castle.Get(world, divineBeast);
                    castles.Add(new Protocol.Castle
                    {
                        DivineBeast = divineBeast,
                        OwnerClanId = castle?.OwnerClan,
                        SiegeId = castle?.SiegeId,
                        SiegeRemaining = castle?.SiegeRemaining(now) ?? 0,
                        SiegeActiveRemaining = castle?.SiegeActiveRemaining(now) ?? 0,
                        SiegeEnded = castle?.SiegeEnded ?? false,
                        Revision = castle?.Revision ?? 0
                    });
                }

                return new Response.CastleList
                {
                    Castles = castles,
                    Error = (uint)ErrorCode.None
                };
            }
            catch (Exception)
            {
                return new Response.CastleList
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost("owner")]
        public async Task<Response.UpdatedCastle> SetOwner(Request.SetCastleOwner request)
        {
            try
            {
                var world = request.World;

                await using var clanLock = request.OwnerClanId.HasValue ? await _distributedLock.Lock(world, Clan.DistributedLockKey(request.OwnerClanId.Value)) : null;
                if (request.OwnerClanId.HasValue)
                {
                    if (await _dbContext.Clan.Get(world, request.OwnerClanId.Value) == null)
                    {
                        throw new LogicException(ErrorCode.NotFoundClan);
                    }
                }

                await using var sync = await _distributedLock.Lock(world, Castle.DistributedLockKey(request.DivineBeast));

                var castle = await Load(world, request.DivineBeast);
                castle.OwnerClan = request.OwnerClanId;
                _dbContext.Castle.Save(world, castle);

                await _dbContext.SaveChangesAsync();

                var response = castle.ToUpdated(request.Host);
                await _rabbitMqService.NotifySavedAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("castle", world));
                return response;
            }
            catch (LogicException e)
            {
                return new Response.UpdatedCastle
                {
                    Host = request.Host,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedCastle
                {
                    Host = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost("siege/start")]
        public async Task<Response.UpdatedCastle> StartSiege(Request.StartCastleSiege request)
        {
            try
            {
                var world = request.World;
                await using var sync = await _distributedLock.Lock(world, Castle.DistributedLockKey(request.DivineBeast));

                var now = DateTime.UtcNow;
                var castle = await Load(world, request.DivineBeast);
                if (castle.SiegeKey == request.SiegeKey || castle.SiegeActive(now))
                {
                    return castle.ToUpdated(request.Host);
                }

                castle.SiegeId = (castle.SiegeId ?? 0) + 1;
                castle.SiegeKey = request.SiegeKey;
                castle.SiegeEndAt = now.AddSeconds(Random.Shared.Next(MinSiegeSeconds, MaxSiegeSeconds + 1));
                castle.SiegeEnded = false;
                _dbContext.Castle.Save(world, castle);
                await _dbContext.SaveChangesAsync();

                var response = castle.ToUpdated(request.Host, true);
                await _rabbitMqService.NotifySavedAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("castle", world));
                return response;
            }
            catch (Exception)
            {
                return new Response.UpdatedCastle
                {
                    Host = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost("siege/end")]
        public async Task<Response.UpdatedCastle> EndSiege(Request.EndCastleSiege request)
        {
            try
            {
                var world = request.World;
                await using var clanLock = request.WinnerClanId.HasValue ? await _distributedLock.Lock(world, Clan.DistributedLockKey(request.WinnerClanId.Value)) : null;
                var winnerExists = request.WinnerClanId.HasValue && await _dbContext.Clan.Get(world, request.WinnerClanId.Value) != null;

                await using var sync = await _distributedLock.Lock(world, Castle.DistributedLockKey(request.DivineBeast));

                var castle = await Load(world, request.DivineBeast);
                if (castle.SiegeId != request.SiegeId || castle.SiegeEnded)
                {
                    return castle.ToUpdated(request.Host);
                }

                if (winnerExists && castle.SiegeActive(DateTime.UtcNow))
                {
                    castle.OwnerClan = request.WinnerClanId;
                }
                castle.SiegeEnded = true;
                _dbContext.Castle.Save(world, castle);
                await _dbContext.SaveChangesAsync();

                var response = castle.ToUpdated(request.Host);
                await _rabbitMqService.NotifySavedAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("castle", world));
                return response;
            }
            catch (Exception)
            {
                return new Response.UpdatedCastle
                {
                    Host = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost("siege/finish")]
        public async Task<Response.UpdatedCastle> FinishSiege(Request.FinishCastleSiege request)
        {
            try
            {
                var world = request.World;
                await using var sync = await _distributedLock.Lock(world, Castle.DistributedLockKey(request.DivineBeast));

                var now = DateTime.UtcNow;
                var castle = await Load(world, request.DivineBeast);
                if (castle.SiegeActive(now) == false || castle.SiegeEndAt <= now)
                {
                    return castle.ToUpdated(request.Host);
                }

                castle.SiegeEndAt = now;
                _dbContext.Castle.Save(world, castle);
                await _dbContext.SaveChangesAsync();

                var response = castle.ToUpdated(request.Host);
                await _rabbitMqService.NotifySavedAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("castle", world));
                return response;
            }
            catch (Exception)
            {
                return new Response.UpdatedCastle
                {
                    Host = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}
