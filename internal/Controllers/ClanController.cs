using AutoMapper;
using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Medallion.Threading.Redis;
using Microsoft.AspNetCore.Mvc;
using StackExchange.Redis;
using Protocol = fb.protocol._internal;
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
        private readonly RabbitMqService _rabbitMqService;
        private readonly IMapper _mapper;

        public ClanController(IConfiguration configuration,
            DbContext dbContext,
            RedisService redisService,
            RabbitMqService rabbitMqService,
            IMapper mapper)
        {
            _configuration = configuration;
            _dbContext = dbContext;
            _redisService = redisService;
            _rabbitMqService = rabbitMqService;
            _mapper = mapper;
        }

        private async Task<List<Protocol.ClanMember>> GetClanMemberResponse(uint id)
        {
            var members = await _dbContext.ClanMember.Get(id);
            var conn = _dbContext.Connection(-1);
            var names = await conn.QueryAsync($"SELECT `id`, `name` FROM `name` WHERE id IN ({string.Join(',', members.Select(x => x.User))})");
            var nameDict = names.ToDictionary(x => x.id, x => x.name);

            return members.Select(x =>
            {
                return new Protocol.ClanMember
                {
                    Uid = x.User,
                    Name = nameDict.GetValueOrDefault(x.User),
                    Position = x.Position
                };
            }).ToList();
        }

        [HttpGet("{id}")]
        public async Task<Response.GetClan> Get(uint id)
        {
            try
            {
                var redis = _redisService.Connection;
                await using (await new RedisDistributedLock(Clan.DistributedLockKey(id), redis).AcquireAsync())
                {
                    var clan = await _dbContext.Clan.Get(id) ??
                        throw new LogicException(ErrorCode.NotFoundClan);

                    var members = await _dbContext.ClanMember.Get(id);
                    var conn = _dbContext.Connection(-1);
                    var names = await conn.QueryAsync($"SELECT `id`, `name` FROM `name` WHERE id IN ({string.Join(',', members.Select(x => x.User))})");
                    var nameDict = names.ToDictionary(x => x.id, x => x.name);

                    return new Response.GetClan
                    {
                        Clan = _mapper.Map<Protocol.Clan>(clan),
                        Members = await GetClanMemberResponse(id)
                    };
                }
            }
            catch (LogicException e)
            {
                return new Response.GetClan
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.GetClan
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }


        [HttpPost("create")]
        public async Task<Response.CreateClan> Create(Request.CreateClan request)
        {
            await using var db = _dbContext.Connection(-1);
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            var redis = _redisService.Connection;
            try
            {
                var ch = await _dbContext.Character.Get(request.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var result = await db.QueryFirstAsync<ClanNameSetResult>("USP_CLAN_NAME_SET", new
                {
                    name = request.Name
                }, transaction: trans, commandType: System.Data.CommandType.StoredProcedure);

                if (!result.Result)
                    throw new LogicException(ErrorCode.ClanNameAlreadyExists);

                await using (await new RedisDistributedLock(CharacterSync.DistributedLockKey(ch.Id), redis).AcquireAsync())
                {
                    await using (await new RedisDistributedLock(Clan.DistributedLockKey(result.Id), redis).AcquireAsync())
                    {
                        var sync = await _dbContext.CharacterSync.Get(ch.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        if (sync.Clan != null)
                            throw new LogicException(ErrorCode.ClanAlreadyJoined);

                        var clan = _dbContext.Clan.Set(new Clan
                        {
                            Id = result.Id,
                            Name = request.Name,
                            Title = null,
                            Deleted = false
                        });

                        _dbContext.ClanMember.Set(new ClanMember
                        {
                            Clan = clan.Id,
                            User = request.Master,
                            Position = (uint)ClanPosition.Master
                        });

                        sync.Clan = clan.Id;
                        _dbContext.CharacterSync.Set(sync);

                        await _dbContext.SaveChangesAsync();
                        await trans.CommitAsync();
                        return new Response.CreateClan
                        {
                            Clan = _mapper.Map<Protocol.Clan>(clan),
                            Members = await GetClanMemberResponse(clan.Id),
                            Error = (uint)ErrorCode.None
                        };
                    }
                }
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
            try
            {
                await using (await new RedisDistributedLock(CharacterSync.DistributedLockKey(request.Master), redis).AcquireAsync())
                {
                    var ch = await _dbContext.Character.Get(request.Master) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var sync = await _dbContext.CharacterSync.Get(ch.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (sync.Clan == null)
                        throw new LogicException(ErrorCode.ClanNotJoined);

                    await using (await new RedisDistributedLock(Clan.DistributedLockKey(sync.Clan.Value), redis).AcquireAsync())
                    {
                        var clan = await _dbContext.Clan.Get(sync.Clan.Value) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        var members = (await _dbContext.ClanMember.Get(clan.Id)).ToList();
                        if (members.Count != 1)
                            throw new LogicException(ErrorCode.ClanMemberExists);

                        var master = members[0];
                        if (master.User != ch.Id)
                            throw new LogicException(ErrorCode.ClanNoPrivilege);

                        if (master.Position != (uint)ClanPosition.Master)
                            throw new LogicException(ErrorCode.ClanNoPrivilege);

                        master.Deleted = true;
                        _dbContext.ClanMember.Set(master);

                        sync.Clan = null;
                        _dbContext.CharacterSync.Set(sync);

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
                }
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
                await db.CloseAsync();
            }
        }

        [HttpPost("title")]
        public async Task<Response.SetClanTitle> SetTitle(Request.SetClanTitle request)
        {
            var redis = _redisService.Connection;
            try
            {
                await using (await new RedisDistributedLock(Clan.DistributedLockKey(request.Clan), redis).AcquireAsync())
                {
                    var clan = await _dbContext.Clan.Get(request.Clan) ??
                        throw new LogicException(ErrorCode.NotFoundClan);

                    if (clan.Title == request.Title)
                        throw new LogicException(ErrorCode.ClanTitleNotChanged);

                    if (request.Title != null && request.Title.Length < 2)
                        throw new LogicException(ErrorCode.ClanTitleTooShort);

                    clan.Title = request.Title;
                    _dbContext.Clan.Set(clan);

                    await _dbContext.SaveChangesAsync();

                    var response = new Response.SetClanTitle
                    {
                        Clan = clan.Id,
                        Title = request.Title,
                        Error = (uint)ErrorCode.None
                    };
                    _rabbitMqService.Publish(response, "amq.direct", $"fb.clan");
                    return response;
                }
            }
            catch (LogicException e)
            {
                return new Response.SetClanTitle
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
            {
                return new Response.SetClanTitle
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }

        [HttpPost("join")]
        public async Task<Response.JoinClan> Join(Request.JoinClan request)
        {
            try
            {
                var redis = _redisService.Connection;
                await using (await new RedisDistributedLock(CharacterSync.DistributedLockKey(request.Uid), redis).AcquireAsync())
                {
                    var target = await _dbContext.Character.Get(request.Uid) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (targetSync.Clan != null)
                        throw new LogicException(ErrorCode.ClanAlreadyJoined);

                    await using (await new RedisDistributedLock(Clan.DistributedLockKey(request.Clan), redis).AcquireAsync())
                    {
                        var clan = await _dbContext.Clan.Get(request.Clan) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        var cm = _dbContext.ClanMember.Set(new ClanMember
                        {
                            Clan = clan.Id,
                            Position = (uint)ClanPosition.Mate,
                            User = target.Id,
                            Deleted = false
                        });

                        targetSync.Clan = clan.Id;
                        _dbContext.CharacterSync.Set(targetSync);

                        await _dbContext.SaveChangesAsync();
                        var response = new Response.JoinClan
                        {
                            Clan = clan.Id,
                            Member = new Protocol.ClanMember
                            {
                                Name = target.Name,
                                Uid = cm.User,
                                Position = cm.Position
                            },
                            Error = (uint)ErrorCode.None
                        };

                        _rabbitMqService.Publish(response, "amq.direct", $"fb.clan");
                        return response;
                    }
                }
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
                var uid = await _dbContext.Character.GetCharacterId(request.Name) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var redis = _redisService.Connection;
                await using (await new RedisDistributedLock(CharacterSync.DistributedLockKey(uid), redis).AcquireAsync())
                {
                    var ch = await _dbContext.Character.Get(uid) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var sync = await _dbContext.CharacterSync.Get(ch.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (sync.Clan == null)
                        throw new LogicException(ErrorCode.ClanNotJoined);

                    if (sync.Clan.Value != request.Clan)
                        throw new LogicException(ErrorCode.ClanNotMatched);

                    await using (await new RedisDistributedLock(Clan.DistributedLockKey(sync.Clan.Value), redis).AcquireAsync())
                    {
                        var clan = await _dbContext.Clan.Get(sync.Clan.Value) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        var member = await _dbContext.ClanMember.Get(clan.Id, ch.Id) ??
                            throw new LogicException(ErrorCode.NotFoundClanMember);

                        if (member.Position == (uint)ClanPosition.Master)
                            throw new LogicException(ErrorCode.ClanCannotLeaveeMaster);

                        member.Deleted = true;
                        _dbContext.ClanMember.Set(member);

                        sync.Clan = null;
                        _dbContext.CharacterSync.Set(sync);

                        await _dbContext.SaveChangesAsync();

                        var response = new Response.LeaveClan
                        {
                            Clan = clan.Id,
                            Uid = ch.Id,
                            Uname = ch.Name,
                            Kick = request.Kick,
                            Error = (uint)ErrorCode.None
                        };

                        _rabbitMqService.Publish(response, "amq.direct", $"fb.clan");
                        return response;
                    }
                }
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

        [HttpPost("broadcast")]
        public async Task<Response.BroadcastClan> Broadcast(Request.BroadcastClan request)
        {
            try
            {
                var redis = _redisService.Connection;
                await using (await new RedisDistributedLock(Clan.DistributedLockKey(request.Clan), redis).AcquireAsync())
                {
                    var clan = await _dbContext.Clan.Get(request.Clan) ??
                        throw new LogicException(ErrorCode.NotFoundClan);

                    var response = new Response.BroadcastClan
                    {
                        Clan = request.Clan,
                        Message = request.Message,
                        Type = request.Type,
                        Error = (uint)ErrorCode.None
                    };
                    _rabbitMqService.Publish(response, "amq.direct", $"fb.clan");
                    return response;
                }
            }
            catch (LogicException e)
            {
                return new Response.BroadcastClan
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.BroadcastClan
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }
    }
}
