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
    [ApiController]
    [Route("castle")]
    public class CastleController : ControllerBase
    {
        private static readonly byte[] AllDivineBeasts = { 0, 1, 2, 3 };
        private const int MinSiegeSeconds = 1800;
        private const int MaxSiegeSeconds = 3600;
        public static readonly TimeSpan SiegeGrace = TimeSpan.FromMinutes(5);

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

        private static bool SiegeActive(Castle castle, DateTime now)
        {
            return castle.SiegeId.HasValue && castle.SiegeEnded == false && castle.SiegeEndAt.HasValue && castle.SiegeEndAt.Value + SiegeGrace > now;
        }

        private static uint SiegeRemaining(Castle castle, DateTime now)
        {
            if (castle.SiegeEndAt.HasValue == false || castle.SiegeEndAt.Value <= now)
            {
                return 0;
            }

            return (uint)Math.Ceiling((castle.SiegeEndAt.Value - now).TotalSeconds);
        }

        private static Response.UpdatedCastle Updated(uint host, Castle castle, bool started = false)
        {
            var now = DateTime.UtcNow;
            return new Response.UpdatedCastle
            {
                Host = host,
                DivineBeast = castle.DivineBeast,
                OwnerClanId = castle.OwnerClan,
                SiegeId = castle.SiegeId,
                SiegeRemaining = SiegeRemaining(castle, now),
                SiegeEnded = castle.SiegeEnded,
                SiegeStarted = started,
                Error = (uint)ErrorCode.None
            };
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
                        SiegeRemaining = castle != null ? SiegeRemaining(castle, now) : 0,
                        SiegeEnded = castle?.SiegeEnded ?? false
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
                _dbContext.Castle.Set(world, castle);

                await _dbContext.SaveChangesAsync();

                var response = Updated(request.Host, castle);
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
                if (castle.SiegeKey == request.SiegeKey || SiegeActive(castle, now))
                {
                    return Updated(request.Host, castle);
                }

                castle.SiegeId = (castle.SiegeId ?? 0) + 1;
                castle.SiegeKey = request.SiegeKey;
                castle.SiegeEndAt = now.AddSeconds(Random.Shared.Next(MinSiegeSeconds, MaxSiegeSeconds + 1));
                castle.SiegeEnded = false;
                _dbContext.Castle.Set(world, castle);
                await _dbContext.SaveChangesAsync();

                var response = Updated(request.Host, castle, true);
                await _rabbitMqService.NotifySavedAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("castle", world));
                if (string.IsNullOrEmpty(request.Announcement) == false)
                {
                    await _rabbitMqService.NotifySavedAsync(new Response.Broadcast
                    {
                        Host = uint.MaxValue,
                        Message = request.Announcement,
                        Type = (byte)MessageType.World,
                        Error = (uint)ErrorCode.None
                    }, AmqpRoute.Exchange, AmqpRoute.Home("global", world));
                }
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
                    return Updated(request.Host, castle);
                }

                if (winnerExists)
                {
                    castle.OwnerClan = request.WinnerClanId;
                }
                castle.SiegeEnded = true;
                _dbContext.Castle.Set(world, castle);
                await _dbContext.SaveChangesAsync();

                var response = Updated(request.Host, castle);
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
                if (SiegeActive(castle, now) == false || castle.SiegeEndAt <= now)
                {
                    return Updated(request.Host, castle);
                }

                castle.SiegeEndAt = now;
                _dbContext.Castle.Set(world, castle);
                await _dbContext.SaveChangesAsync();

                var response = Updated(request.Host, castle);
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
