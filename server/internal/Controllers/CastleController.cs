using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("castle")]
    public class CastleController : ControllerBase
    {
        private static readonly byte[] AllDivineBeasts = { 0, 1, 2, 3 };

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

        [HttpGet("{world}")]
        public async Task<Response.CastleList> Get(uint world)
        {
            try
            {
                var castles = new List<Protocol.Castle>();
                foreach (var divineBeast in AllDivineBeasts)
                {
                    var castle = await _dbContext.Castle.Get(world, divineBeast);
                    castles.Add(new Protocol.Castle
                    {
                        DivineBeast = divineBeast,
                        OwnerClanId = castle?.OwnerClan
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

                if (request.OwnerClanId.HasValue)
                {
                    if (await _dbContext.Clan.Get(world, request.OwnerClanId.Value) == null)
                        throw new LogicException(ErrorCode.NotFoundClan);
                }

                await using var sync = await _distributedLock.Lock(world, Castle.DistributedLockKey(request.DivineBeast));

                _dbContext.Castle.Set(world, new Castle
                {
                    DivineBeast = request.DivineBeast,
                    OwnerClan = request.OwnerClanId
                });

                await _dbContext.SaveChangesAsync();

                var response = new Response.UpdatedCastle
                {
                    Host = request.Host,
                    DivineBeast = request.DivineBeast,
                    OwnerClanId = request.OwnerClanId,
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.PublishAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("castle", world));
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
    }
}
