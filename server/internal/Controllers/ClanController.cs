using AutoMapper;
using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
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
        private readonly RedisDistributedLockService _distributedLock;
        private readonly SessionService _sessionService;
        public ClanController(IConfiguration configuration,
            DbContext dbContext,
            RedisService redisService,
            RabbitMqService rabbitMqService,
            IMapper mapper,
            RedisDistributedLockService distributedLock,
            SessionService sessionService)
        {
            _configuration = configuration;
            _dbContext = dbContext;
            _redisService = redisService;
            _rabbitMqService = rabbitMqService;
            _mapper = mapper;
            _distributedLock = distributedLock;
            _sessionService = sessionService;
        }
        private async Task<List<Protocol.ClanMember>> GetClanMemberResponse(string section, uint id)
        {
            var members = await _dbContext.ClanMember.Get(section, id);
            await using var conn = _dbContext.Connection(section, -1);
            var names = await conn.QueryAsync($"SELECT `id`, `name` FROM `name` WHERE id IN ({string.Join(',', members.Select(x => x.User))})");
            var nameDict = names.ToDictionary(x => x.id, x => x.name);

            return members.Select(x =>
            {
                return new Protocol.ClanMember
                {
                    Uid = x.User,
                    Name = nameDict.GetValueOrDefault(x.User),
                    Role = x.Role
                };
            }).ToList();
        }

        [HttpGet("{section}/{id}")]
        public async Task<Response.ClanDetails> Get(string section, uint id)
        {
            try
            {
                await using (await _distributedLock.Lock(Clan.DistributedLockKey(id)))
                {
                    var clan = await _dbContext.Clan.Get(section, id) ??
                        throw new LogicException(ErrorCode.NotFoundClan);

                    return new Response.ClanDetails
                    {
                        Action = Protocol.ClanDetailsAction.Query,
                        Clan = _mapper.Map<Protocol.Clan>(clan),
                        Members = await GetClanMemberResponse(section, id),
                        Error = (uint)ErrorCode.None
                    };
                }
            }
            catch (LogicException e)
            {
                return new Response.ClanDetails
                {
                    Action = Protocol.ClanDetailsAction.Query,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.ClanDetails
                {
                    Action = Protocol.ClanDetailsAction.Query,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }
        [HttpPost("create")]
        public async Task<Response.ClanDetails> Create(Request.CreateClan request)
        {
            var section = request.Section;
            await using var db = _dbContext.Connection(section, -1);
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            try
            {
                var ch = await _dbContext.Character.Get(section, request.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var existing = await db.QueryFirstOrDefaultAsync<uint?>(
                    @"SELECT id FROM clan_name WHERE name = @Name FOR UPDATE",
                    new { Name = request.Name },
                    transaction: trans
                );
                if (existing.HasValue)
                    throw new LogicException(ErrorCode.ClanNameAlreadyExists);

                var newClanId = await db.QuerySingleAsync<uint>(
                    @"INSERT INTO clan_name (name) VALUES (@Name);SELECT LAST_INSERT_ID();",
                    new { Name = request.Name },
                    transaction: trans
                );

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(ch.Id)))
                {
                    await using (await _distributedLock.Lock(Clan.DistributedLockKey(newClanId)))
                    {
                        var sync = await _dbContext.CharacterSync.Get(section, ch.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        if (sync.Clan != null)
                            throw new LogicException(ErrorCode.ClanAlreadyJoined);

                        var clan = _dbContext.Clan.Set(section, new Clan
                        {
                            Id = newClanId,
                            Name = request.Name,
                            Title = null,
                            Deleted = false
                        });

                        _dbContext.ClanMember.Set(section, new ClanMember
                        {
                            Clan = clan.Id,
                            User = request.Master,
                            Role = (uint)ClanRole.Master
                        });

                        sync.Clan = clan.Id;
                        _dbContext.CharacterSync.Set(section, sync);

                        await _dbContext.SaveChangesAsync();
                        await trans.CommitAsync();

                        var response = new Response.ClanDetails
                        {
                            Host = request.Host,
                            Action = Protocol.ClanDetailsAction.Create,
                            Clan = _mapper.Map<Protocol.Clan>(clan),
                            Members = await GetClanMemberResponse(section, clan.Id),
                            Error = (uint)ErrorCode.None
                        };

                        _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.clan");
                        return response;
                    }
                }
            }
            catch (LogicException e)
            {
                await trans.RollbackAsync();
                return new Response.ClanDetails
                {
                    Host = request.Host,
                    Action = Protocol.ClanDetailsAction.Create,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                await trans.RollbackAsync();
                return new Response.ClanDetails
                {
                    Host = request.Host,
                    Action = Protocol.ClanDetailsAction.Create,
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
            var section = request.Section;
            await using var db = _dbContext.Connection(section, -1);
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            try
            {
                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(request.Master)))
                {
                    var ch = await _dbContext.Character.Get(section, request.Master) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var sync = await _dbContext.CharacterSync.Get(section, ch.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (sync.Clan == null)
                        throw new LogicException(ErrorCode.ClanNotJoined);

                    await using (await _distributedLock.Lock(Clan.DistributedLockKey(sync.Clan.Value)))
                    {
                        var clan = await _dbContext.Clan.Get(section, sync.Clan.Value) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        var members = (await _dbContext.ClanMember.Get(section, clan.Id)).ToList();
                        if (members.Count != 1)
                            throw new LogicException(ErrorCode.ClanMemberExists);

                        var master = members[0];
                        if (master.User != ch.Id)
                            throw new LogicException(ErrorCode.ClanNoPrivilege);

                        if (master.Role != (uint)ClanRole.Master)
                            throw new LogicException(ErrorCode.ClanNoPrivilege);

                        master.Deleted = true;
                        _dbContext.ClanMember.Set(section, master);

                        sync.Clan = null;
                        _dbContext.CharacterSync.Set(section, sync);

                        var oldTitle = clan.Title;
                        clan.Deleted = true;
                        _dbContext.Clan.Set(section, clan);

                        await db.ExecuteAsync("USP_CLAN_NAME_DELETE", new
                        {
                            id = clan.Id
                        }, transaction: trans, commandType: System.Data.CommandType.StoredProcedure);

                        await _dbContext.SaveChangesAsync();
                        await trans.CommitAsync();

                        var conn = _dbContext.Connection(section, -1);
                        var masterName = await conn.QueryFirstOrDefaultAsync<string>(
                            $"SELECT `name` FROM `name` WHERE id = {ch.Id}");

                        var response = new Response.DestroyClan
                        {
                            Host = request.Host,
                            ClanId = clan.Id,
                            ClanName = clan.Name,
                            Actor = new Protocol.CharacterRef
                            {
                                Uid = ch.Id,
                                Name = masterName ?? ch.Name
                            },
                            Error = (uint)ErrorCode.None
                        };

                        _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.clan");
                        return response;
                    }
                }
            }
            catch (LogicException e)
            {
                await trans.RollbackAsync();
                return new Response.DestroyClan
                {
                    Host = request.Host,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                await trans.RollbackAsync();
                return new Response.DestroyClan
                {
                    Host = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            {
                await db.CloseAsync();
            }
        }
        [HttpPost("title")]
        public async Task<Response.UpdatedClan> SetTitle(Request.SetClanTitle request)
        {
            try
            {
                var section = request.Section;
                // Get changer character
                var changer = await _dbContext.Character.Get(section, request.Changer) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(changer.Id)))
                {
                    var sync = await _dbContext.CharacterSync.Get(section, changer.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (sync.Clan == null)
                        throw new LogicException(ErrorCode.ClanNotJoined);

                    await using (await _distributedLock.Lock(Clan.DistributedLockKey(sync.Clan.Value)))
                    {
                        var clan = await _dbContext.Clan.Get(section, sync.Clan.Value) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        // Get changer's clan member info
                        var changerMember = await _dbContext.ClanMember.Get(section, clan.Id, changer.Id) ??
                            throw new LogicException(ErrorCode.NotFoundClanMember);

                        // Check if changer has sufficient privileges (Master role or higher)
                        if (changerMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumChangeTitlePrivilege)
                            throw new LogicException(ErrorCode.ClanNoPrivilege);

                        if (clan.Title == request.Title)
                            throw new LogicException(ErrorCode.ClanTitleNotChanged);

                        if (request.Title != null && request.Title.Length < 2)
                            throw new LogicException(ErrorCode.ClanTitleTooShort);

                        clan.Title = request.Title;
                        _dbContext.Clan.Set(section, clan);

                        await _dbContext.SaveChangesAsync();

                        var response = new Response.UpdatedClan
                        {
                            Host = request.Host,
                            Action = Protocol.ClanActionType.SetTitle,
                            ClanId = clan.Id,
                            ClanName = clan.Name,
                            Actor = new Protocol.CharacterRef
                            {
                                Uid = changer.Id,
                                Name = changer.Name
                            },
                            OldTitle = clan.Title,
                            NewTitle = request.Title,
                            Error = (uint)ErrorCode.None
                        };
                        _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.clan");
                        return response;
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.SetTitle,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.SetTitle,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }
        [HttpPost("join")]
        public async Task<Response.UpdatedClan> Join(Request.JoinClan request)
        {
            try
            {
                var section = request.Section;
                // Get inviter character (the one doing the inviting)
                var inviter = await _dbContext.Character.Get(section, request.InviterUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (inviter.Name == request.InviteeName)
                    throw new LogicException(ErrorCode.CannotInviteSelf);

                // Get invitee session and character (the one being invited)
                var inviteeSession = await _sessionService.Get(section, request.InviteeName) ??
                    throw new LogicException(ErrorCode.Offline);

                var invitee = await _dbContext.Character.Get(section, inviteeSession.Uid) ??
                    throw new LogicException(ErrorCode.Offline);

                // Prevent self-invitation
                if (inviter.Id == invitee.Id)
                    throw new LogicException(ErrorCode.CannotInviteSelf);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(inviter.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(invitee.Id)))
                    {
                        var inviterSync = await _dbContext.CharacterSync.Get(section, inviter.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var inviteeSync = await _dbContext.CharacterSync.Get(section, invitee.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Check if inviter is in a clan
                        if (inviterSync.Clan == null)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        // Check if invitee is already in a clan
                        if (inviteeSync.Clan != null)
                            throw new LogicException(ErrorCode.ClanAlreadyJoined);

                        await using (await _distributedLock.Lock(Clan.DistributedLockKey(inviterSync.Clan.Value)))
                        {
                            var clan = await _dbContext.Clan.Get(section, inviterSync.Clan.Value) ??
                                throw new LogicException(ErrorCode.NotFoundClan);

                            // Get inviter's clan member info
                            var inviterMember = await _dbContext.ClanMember.Get(section, clan.Id, inviter.Id) ??
                                throw new LogicException(ErrorCode.NotFoundClanMember);

                            // Check if inviter has sufficient privileges
                            if (inviterMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumInvitePrivilege)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Add invitee to clan
                            var cm = _dbContext.ClanMember.Set(section, new ClanMember
                            {
                                Clan = clan.Id,
                                Role = (uint)ClanRole.Mate,
                                User = invitee.Id,
                                Deleted = false
                            });

                            inviteeSync.Clan = clan.Id;
                            _dbContext.CharacterSync.Set(section, inviteeSync);

                            await _dbContext.SaveChangesAsync();

                            var response = new Response.UpdatedClan
                            {
                                Host = request.Host,
                                Action = Protocol.ClanActionType.Join,
                                ClanId = clan.Id,
                                ClanName = clan.Name,
                                Actor = new Protocol.CharacterRef
                                {
                                    Uid = inviter.Id,
                                    Name = inviter.Name
                                },
                                Target = new Protocol.CharacterRef
                                {
                                    Uid = invitee.Id,
                                    Name = invitee.Name
                                },
                                NewMember = new Protocol.CharacterRef
                                {
                                    Uid = invitee.Id,
                                    Name = invitee.Name
                                },
                                Error = (uint)ErrorCode.None
                            };

                            _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.clan");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Join,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Join,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("leave")]
        public async Task<Response.UpdatedClan> Leave(Request.LeaveClan request)
        {
            try
            {
                var section = request.Section;
                var uid = await _dbContext.Character.GetCharacterId(section, request.Name) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(uid)))
                {
                    var ch = await _dbContext.Character.Get(section, uid) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var sync = await _dbContext.CharacterSync.Get(section, ch.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (sync.Clan == null)
                        throw new LogicException(ErrorCode.ClanNotJoined);

                    if (sync.Clan.Value != request.Clan)
                        throw new LogicException(ErrorCode.ClanNotMatched);

                    await using (await _distributedLock.Lock(Clan.DistributedLockKey(sync.Clan.Value)))
                    {
                        var clan = await _dbContext.Clan.Get(section, sync.Clan.Value) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        var member = await _dbContext.ClanMember.Get(section, clan.Id, ch.Id) ??
                            throw new LogicException(ErrorCode.NotFoundClanMember);

                        if (member.Role == (uint)ClanRole.Master)
                            throw new LogicException(ErrorCode.ClanCannotLeaveeMaster);

                        member.Deleted = true;
                        _dbContext.ClanMember.Set(section, member);

                        sync.Clan = null;
                        _dbContext.CharacterSync.Set(section, sync);

                        await _dbContext.SaveChangesAsync();

                        var response = new Response.UpdatedClan
                        {
                            Host = request.Host,
                            Action = Protocol.ClanActionType.Leave,
                            ClanId = clan.Id,
                            ClanName = clan.Name,
                            Actor = new Protocol.CharacterRef
                            {
                                Uid = ch.Id,
                                Name = ch.Name
                            },
                            Target = new Protocol.CharacterRef
                            {
                                Uid = ch.Id,
                                Name = ch.Name
                            },
                            DeletedMember = new Protocol.CharacterRef
                            {
                                Uid = ch.Id,
                                Name = ch.Name
                            },
                            Error = (uint)ErrorCode.None
                        };

                        _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.clan");
                        return response;
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Leave,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Leave,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }
        [HttpPost("kick")]
        public async Task<Response.UpdatedClan> Kick(Request.KickClan request)
        {
            try
            {
                var section = request.Section;
                // Get kicker character
                var kickerUid = await _dbContext.Character.GetCharacterId(section, request.Kicker) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var kicker = await _dbContext.Character.Get(section, kickerUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Get target character
                var targetUid = await _dbContext.Character.GetCharacterId(section, request.Target) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var target = await _dbContext.Character.Get(section, targetUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Prevent self-kicking
                if (kicker.Name == target.Name)
                    throw new LogicException(ErrorCode.CannotKickSelf);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(kicker.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var kickerSync = await _dbContext.CharacterSync.Get(section, kicker.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(section, target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Check if kicker is in a clan
                        if (kickerSync.Clan == null)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        // Check if target is in the same clan
                        if (targetSync.Clan != kickerSync.Clan)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        await using (await _distributedLock.Lock(Clan.DistributedLockKey(kickerSync.Clan.Value)))
                        {
                            var clan = await _dbContext.Clan.Get(section, kickerSync.Clan.Value) ??
                                throw new LogicException(ErrorCode.NotFoundClan);

                            // Get kicker's clan member info
                            var kickerMember = await _dbContext.ClanMember.Get(section, clan.Id, kicker.Id) ??
                                throw new LogicException(ErrorCode.NotFoundClanMember);

                            // Get target's clan member info
                            var targetMember = await _dbContext.ClanMember.Get(section, clan.Id, target.Id) ??
                                throw new LogicException(ErrorCode.NotFoundClanMember);

                            // Check if kicker has minimum kickable role
                            if (kickerMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumKickPrivilege)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Check if target has higher or equal role (can't kick superiors)
                            if (targetMember.Role >= kickerMember.Role)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Check if target is not the master (can't kick master)
                            if (targetMember.Role == (uint)ClanRole.Master)
                                throw new LogicException(ErrorCode.ClanCannotLeaveeMaster);

                            // Remove target from clan
                            targetMember.Deleted = true;
                            _dbContext.ClanMember.Set(section, targetMember);

                            targetSync.Clan = null;
                            _dbContext.CharacterSync.Set(section, targetSync);

                            await _dbContext.SaveChangesAsync();

                            var response = new Response.UpdatedClan
                            {
                                Host = request.Host,
                                Action = Protocol.ClanActionType.Kick,
                                ClanId = clan.Id,
                                ClanName = clan.Name,
                                Actor = new Protocol.CharacterRef
                                {
                                    Uid = kicker.Id,
                                    Name = kicker.Name
                                },
                                Target = new Protocol.CharacterRef
                                {
                                    Uid = target.Id,
                                    Name = target.Name
                                },
                                DeletedMember = new Protocol.CharacterRef
                                {
                                    Uid = target.Id,
                                    Name = target.Name
                                },
                                Error = (uint)ErrorCode.None
                            };

                            _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.clan");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Kick,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Kick,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("broadcast")]
        public async Task<Response.BroadcastClan> Broadcast(Request.BroadcastClan request)
        {
            try
            {
                var section = request.Section;
                await using (await _distributedLock.Lock(Clan.DistributedLockKey(request.Clan)))
                {
                    var clan = await _dbContext.Clan.Get(section, request.Clan) ??
                        throw new LogicException(ErrorCode.NotFoundClan);

                    var response = new Response.BroadcastClan
                    {
                        Clan = request.Clan,
                        Message = request.Message,
                        Type = request.Type,
                        Error = (uint)ErrorCode.None
                    };
                    _rabbitMqService.Publish(section, response, "amq.direct", $"fb.clan");
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
        [HttpPost("change-role")]
        public async Task<Response.UpdatedClan> ChangeRole(Request.ChangeClanRole request)
        {
            try
            {
                var section = request.Section;
                // Get changer character (the one making the change)
                var changer = await _dbContext.Character.Get(section, request.ChangerUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Get target character (the one being changed)
                var targetUid = await _dbContext.Character.GetCharacterId(section, request.TargetName) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var target = await _dbContext.Character.Get(section, targetUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Prevent self-role change
                if (changer.Id == target.Id)
                    throw new LogicException(ErrorCode.CannotChangeClanRoleSelf);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(changer.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var changerSync = await _dbContext.CharacterSync.Get(section, changer.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(section, target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Check if changer is in a clan
                        if (changerSync.Clan == null)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        // Check if target is in the same clan
                        if (targetSync.Clan != changerSync.Clan)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        await using (await _distributedLock.Lock(Clan.DistributedLockKey(changerSync.Clan.Value)))
                        {
                            var clan = await _dbContext.Clan.Get(section, changerSync.Clan.Value) ??
                                throw new LogicException(ErrorCode.NotFoundClan);

                            // Get changer's clan member info
                            var changerMember = await _dbContext.ClanMember.Get(section, clan.Id, changer.Id) ??
                                throw new LogicException(ErrorCode.NotFoundClanMember);

                            // Get target's clan member info
                            var targetMember = await _dbContext.ClanMember.Get(section, clan.Id, target.Id) ??
                                throw new LogicException(ErrorCode.NotFoundClanMember);

                            // Check if changer has sufficient privileges
                            if (changerMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumChangeRolePrivilege)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Check if target has higher or equal role (can't change superiors)
                            if (targetMember.Role >= changerMember.Role)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Check if trying to change master role (only master can change master)
                            if (targetMember.Role == (uint)ClanRole.Master &&
                                changerMember.Role != (uint)ClanRole.Master)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Validate new role
                            if (request.NewRole < (uint)ClanRole.Mate ||
                                request.NewRole > (uint)ClanRole.Master)
                                throw new LogicException(ErrorCode.InvalidClanRole);

                            // Check if new role is higher than changer's role (can't promote to higher rank)
                            if (request.NewRole >= changerMember.Role)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Store old role for response
                            var oldRole = targetMember.Role;

                            // Update target's role
                            targetMember.Role = request.NewRole;
                            _dbContext.ClanMember.Set(section, targetMember);

                            await _dbContext.SaveChangesAsync();

                            var response = new Response.UpdatedClan
                            {
                                Host = request.Host,
                                Action = Protocol.ClanActionType.ChangeRole,
                                ClanId = clan.Id,
                                ClanName = clan.Name,
                                Actor = new Protocol.CharacterRef
                                {
                                    Uid = changer.Id,
                                    Name = changer.Name
                                },
                                Target = new Protocol.CharacterRef
                                {
                                    Uid = target.Id,
                                    Name = target.Name
                                },
                                OldRole = oldRole,
                                NewRole = request.NewRole,
                                Error = (uint)ErrorCode.None
                            };

                            _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.clan");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.ChangeRole,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.ChangeRole,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}
