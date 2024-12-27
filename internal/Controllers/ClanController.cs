using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("clan")]
    public class ClanController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly DbContext _dbContext;
        private readonly RedisService _redisService;

        public ClanController(IConfiguration configuration,
            DbContext dbContext,
            RedisService redisService)
        {
            _configuration = configuration;
            _dbContext = dbContext;
            _redisService = redisService;
        }

        [HttpPost("create")]
        public async Task<Response.CreateClan> Create(Request.CreateClan request)
        {
            await using var db = _dbContext.Connection(-1);
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            var redis = _redisService.Connection;
            var redisLockKey = string.Empty;
            try
            {
                var ch = await _dbContext.Character.Get(request.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (ch.Clan != null)
                    throw new LogicException(ErrorCode.ClanAlreadyJoined);

                var result = await db.QueryFirstAsync<ClanNameSetResult>("USP_CLAN_NAME_SET", new
                {
                    name = request.Name
                }, transaction: trans, commandType: System.Data.CommandType.StoredProcedure);

                if (!result.Result)
                    throw new LogicException(ErrorCode.ClanNameAlreadyExists);

                redisLockKey = $"lock:clan:{result.Id}";
                if (await redis.LockTakeAsync(redisLockKey, Environment.MachineName, TimeSpan.FromSeconds(1)) == false)
                    throw new LogicException(ErrorCode.DistributedLockFailed);

                var clan = _dbContext.Clan.Set(new Clan
                {
                    Id = result.Id,
                    Name = request.Name,
                    Master = request.Master,
                    Title = null,
                    Deleted = false
                });

                _dbContext.ClanMember.Set(new ClanMember
                {
                    Clan = clan.Id,
                    User = request.Master,
                    Position = (uint)ClanPosition.Master
                });

                ch.Clan = clan.Id;
                _dbContext.Character.Set(ch);

                await _dbContext.SaveChangesAsync();
                await trans.CommitAsync();
                return new Response.CreateClan
                {
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                await trans.RollbackAsync();
                return new Response.CreateClan
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                await trans.RollbackAsync();
                return new Response.CreateClan
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            {
                if (string.IsNullOrEmpty(redisLockKey) == false)
                    await redis.LockReleaseAsync(redisLockKey, Environment.MachineName);

                await db.CloseAsync();
            }
        }

        [HttpPost("destroy")]
        public async Task<Response.DestroyClan> Destroy(Request.DestroyClan request)
        {
            await using var db = _dbContext.Connection(-1);
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            var redis = _redisService.Connection;
            var redisLockKey = string.Empty;
            try
            {
                var ch = await _dbContext.Character.Get(request.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (ch.Clan == null)
                    throw new LogicException(ErrorCode.ClanNotJoined);

                redisLockKey = $"lock:clan:{ch.Clan.Value}";
                if (await redis.LockTakeAsync(redisLockKey, Environment.MachineName, TimeSpan.FromSeconds(1)) == false)
                    throw new LogicException(ErrorCode.DistributedLockFailed);

                var clan = await _dbContext.Clan.Get(ch.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                if (clan.Deleted)
                    throw new LogicException(ErrorCode.NotFoundClan);

                var members = (await _dbContext.ClanMember.Get(clan.Id)).Where(x => !x.Deleted).ToList();
                if (members.Count != 1)
                    throw new LogicException(ErrorCode.ClanMemberExists);

                var master = members[0];
                if (master.User != ch.Id)
                    throw new LogicException(ErrorCode.ClanNoPrivilege);

                if (master.Position != (uint)ClanPosition.Master)
                    throw new LogicException(ErrorCode.ClanNoPrivilege);

                master.Deleted = true;
                _dbContext.ClanMember.Set(master);

                ch.Clan = null;
                _dbContext.Character.Set(ch);

                clan.Deleted = true;
                _dbContext.Clan.Set(clan);

                await db.ExecuteAsync("USP_CLAN_NAME_DELETE", new
                {
                    id = clan.Id
                }, transaction: trans, commandType: System.Data.CommandType.StoredProcedure);

                await _dbContext.SaveChangesAsync();
                await trans.CommitAsync();
                return new Response.DestroyClan
                {
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                await trans.RollbackAsync();
                return new Response.DestroyClan
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                await trans.RollbackAsync();
                return new Response.DestroyClan
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            {
                if (string.IsNullOrEmpty(redisLockKey) == false)
                    await redis.LockReleaseAsync(redisLockKey, Environment.MachineName);

                await db.CloseAsync();
            }
        }

        [HttpPost("join")]
        public async Task<Response.JoinClan> Join(Request.JoinClan request)
        {
            try
            {
                var master = await _dbContext.Character.Get(request.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var target = await _dbContext.Character.Get(request.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (master.Clan == null)
                    throw new LogicException(ErrorCode.ClanNotJoined);

                if (target.Clan != null)
                    throw new LogicException(ErrorCode.ClanAlreadyJoined);

                var clan = await _dbContext.Clan.Get(master.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                if (clan.Deleted)
                    throw new LogicException(ErrorCode.NotFoundClan);

                var clanMember = await _dbContext.ClanMember.Get(clan.Id, master.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (clanMember.Deleted)
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (clanMember.Position != (uint)ClanPosition.Master)
                    throw new LogicException(ErrorCode.ClanNoPrivilege);

                _dbContext.ClanMember.Set(new ClanMember
                {
                    Clan = clan.Id,
                    Position = (uint)ClanPosition.Mate,
                    User = target.Id,
                    Deleted = false
                });

                target.Clan = clan.Id;
                _dbContext.Character.Set(target);

                await _dbContext.SaveChangesAsync();
                return new Response.JoinClan
                {
                    Clan = clan.Id,
                    Uid = target.Id,
                    Uname = target.Name,
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                return new Response.JoinClan
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
            {
                return new Response.JoinClan
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            {
            }
        }

        [HttpPost("leave")]
        public async Task<Response.LeaveClan> Leave(Request.LeaveClan request)
        {
            try
            {
                var ch = await _dbContext.Character.Get(request.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (ch.Clan == null)
                    throw new LogicException(ErrorCode.ClanNotJoined);

                var clan = await _dbContext.Clan.Get(ch.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                if (clan.Deleted)
                    throw new LogicException(ErrorCode.NotFoundClan);

                var member = await _dbContext.ClanMember.Get(clan.Id, ch.Id) ??
                        throw new LogicException(ErrorCode.NotFoundClanMember);

                if (member.Deleted)
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (member.Position == (uint)ClanPosition.Master)
                    throw new LogicException(ErrorCode.AlreadyLogin); // TODO: CANNOT_LEAVE_MASTER_POSITION

                member.Deleted = true;
                _dbContext.ClanMember.Set(member);

                ch.Clan = null;
                _dbContext.Character.Set(ch);

                await _dbContext.SaveChangesAsync();

                return new Response.LeaveClan
                {
                    Clan = clan.Id,
                    Uid = ch.Id,
                    Uname = ch.Name,
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                return new Response.LeaveClan
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.LeaveClan
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }

        [HttpPost("kick")]
        public async Task<Response.KickClan> Kick(Request.KickClan request)
        {
            try
            {
                var master = await _dbContext.Character.Get(request.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var target = await _dbContext.Character.Get(request.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (master.Clan != target.Clan)
                    throw new LogicException(ErrorCode.ClanNotMatched);

                if (master.Clan == null)
                    throw new LogicException(ErrorCode.ClanNotJoined);

                var clan = await _dbContext.Clan.Get(master.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                var masterMember = await _dbContext.ClanMember.Get(clan.Id, master.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (masterMember.Deleted)
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (masterMember.Position != (uint)ClanPosition.Master)
                    throw new LogicException(ErrorCode.ClanNoPrivilege);

                var targetMember = await _dbContext.ClanMember.Get(clan.Id, target.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (targetMember.Deleted)
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                targetMember.Deleted = true;
                _dbContext.ClanMember.Set(targetMember);

                target.Clan = null;
                _dbContext.Character.Set(target);

                await _dbContext.SaveChangesAsync();

                return new Response.KickClan
                {
                    Clan = clan.Id,
                    Uid = target.Id,
                    Uname = target.Name,
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                return new Response.KickClan
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.KickClan
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }
    }
}
