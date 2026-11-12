using AutoMapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using System.Globalization;
using Protocol = fb.protocol.@internal;
using Request = fb.protocol.@internal.request;
using Response = fb.protocol.@internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("marriage")]
    public class MarriageController : ControllerBase
    {
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly RedisDistributedLockService _distributedLock;

        public MarriageController(IMapper mapper, DbContext dbContext, RedisDistributedLockService distributedLock)
        {
            _mapper = mapper;
            _dbContext = dbContext;
            _distributedLock = distributedLock;
        }

        private static DateTime ParseUtc(string value)
        {
            return DateTime.Parse(value, CultureInfo.InvariantCulture, DateTimeStyles.AssumeUniversal | DateTimeStyles.AdjustToUniversal);
        }

        private async Task<Marriage> Load(uint world, uint uid)
        {
            return await _dbContext.Marriage.Get(world, uid) ?? new Marriage { CharacterId = uid };
        }

        private async Task<Protocol.Marriage> ToProtocol(uint world, Marriage marriage)
        {
            var protocol = _mapper.Map<Protocol.Marriage>(marriage);
            if (marriage.SpouseId.HasValue)
            {
                protocol.SpouseName = await _dbContext.Character.GetName(world, marriage.SpouseId.Value) ?? string.Empty;
            }

            return protocol;
        }

        [HttpPost("marry")]
        public async Task<Response.UpdatedMarriage> Marry(Request.Marry request)
        {
            try
            {
                var world = request.World;
                await using var _1 = await _distributedLock.Lock(world, Marriage.DistributedLockKey(Math.Min(request.User, request.Spouse)));
                await using var _2 = await _distributedLock.Lock(world, Marriage.DistributedLockKey(Math.Max(request.User, request.Spouse)));

                var user = await Load(world, request.User);
                var spouse = await Load(world, request.Spouse);
                if (user.SpouseId != request.Spouse || spouse.SpouseId != request.User)
                {
                    var remarriageAfter = ParseUtc(request.RemarriageAfter);
                    user.SpouseId = request.Spouse;
                    user.RemarriageAfter = remarriageAfter;
                    spouse.SpouseId = request.User;
                    spouse.RemarriageAfter = remarriageAfter;
                    _dbContext.Marriage.Set(world, user);
                    _dbContext.Marriage.Set(world, spouse);
                    await _dbContext.Marriage.SaveChangesAsync();
                }

                return new Response.UpdatedMarriage
                {
                    User = await ToProtocol(world, user),
                    Spouse = await ToProtocol(world, spouse),
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                return new Response.UpdatedMarriage
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedMarriage
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost("divorce")]
        public async Task<Response.UpdatedMarriage> Divorce(Request.Divorce request)
        {
            try
            {
                var world = request.World;
                await using var _1 = await _distributedLock.Lock(world, Marriage.DistributedLockKey(Math.Min(request.User, request.Spouse)));
                await using var _2 = await _distributedLock.Lock(world, Marriage.DistributedLockKey(Math.Max(request.User, request.Spouse)));

                var user = await Load(world, request.User);
                var spouse = await Load(world, request.Spouse);
                var remarriageAfter = ParseUtc(request.RemarriageAfter);
                if (user.SpouseId == request.Spouse)
                {
                    user.SpouseId = null;
                    user.RemarriageAfter = remarriageAfter;
                    user.DivorceCount++;
                    _dbContext.Marriage.Set(world, user);
                }

                if (spouse.SpouseId == request.User)
                {
                    spouse.SpouseId = null;
                    spouse.RemarriageAfter = remarriageAfter;
                    spouse.DivorceCount++;
                    _dbContext.Marriage.Set(world, spouse);
                }

                await _dbContext.Marriage.SaveChangesAsync();
                return new Response.UpdatedMarriage
                {
                    User = await ToProtocol(world, user),
                    Spouse = await ToProtocol(world, spouse),
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                return new Response.UpdatedMarriage
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedMarriage
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}
