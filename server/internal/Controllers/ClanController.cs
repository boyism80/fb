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
    /// <summary>
    /// Provides clan management operations for the internal API.
    /// Handles clan creation, destruction, member management, title setting, and clan broadcasting.
    /// </summary>
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

        /// <summary>
        /// Initializes a new instance of the <see cref="ClanController"/> class.
        /// </summary>
        /// <param name="configuration">The application configuration.</param>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="rabbitMqService">The RabbitMQ service for inter-server messaging.</param>
        /// <param name="mapper">The AutoMapper instance for object mapping.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        public ClanController(IConfiguration configuration,
            DbContext dbContext,
            RedisService redisService,
            RabbitMqService rabbitMqService,
            IMapper mapper,
            RedisDistributedLockService distributedLock)
        {
            _configuration = configuration;
            _dbContext = dbContext;
            _redisService = redisService;
            _rabbitMqService = rabbitMqService;
            _mapper = mapper;
            _distributedLock = distributedLock;
        }

        /// <summary>
        /// Retrieves clan member information for response formatting.
        /// Helper method that fetches clan members and their names from the database.
        /// </summary>
        /// <param name="id">The clan ID to retrieve members for.</param>
        /// <returns>A list of clan member protocol objects with names and positions.</returns>
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
                    Role = x.Role
                };
            }).ToList();
        }

        /// <summary>
        /// Retrieves clan information by clan ID.
        /// Returns the clan details including member information and positions.
        /// </summary>
        /// <param name="id">The unique identifier of the clan to retrieve.</param>
        /// <returns>A response containing the clan information and member list or error details.</returns>
        [HttpGet("{id}")]
        public async Task<Response.GetClan> Get(uint id)
        {
            try
            {
                await using (await _distributedLock.Lock(Clan.DistributedLockKey(id)))
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

        /// <summary>
        /// Handles clan creation requests with name validation and master assignment.
        /// Creates a new clan with unique name validation and assigns the creator as master.
        /// </summary>
        /// <param name="request">The clan creation request containing master ID and clan name.</param>
        /// <returns>A response with the created clan information or error details.</returns>
        [HttpPost("create")]
        public async Task<Response.CreateClan> Create(Request.CreateClan request)
        {
            await using var db = _dbContext.Connection(-1);
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            try
            {
                var ch = await _dbContext.Character.Get(request.Master) ??
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
                        var sync = await _dbContext.CharacterSync.Get(ch.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        if (sync.Clan != null)
                            throw new LogicException(ErrorCode.ClanAlreadyJoined);

                        var clan = _dbContext.Clan.Set(new Clan
                        {
                            Id = newClanId,
                            Name = request.Name,
                            Title = null,
                            Deleted = false
                        });

                        _dbContext.ClanMember.Set(new ClanMember
                        {
                            Clan = clan.Id,
                            User = request.Master,
                            Role = (uint)ClanRole.Master
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

        /// <summary>
        /// Handles clan destruction requests by the clan master.
        /// Validates master privileges and removes the clan if no other members exist.
        /// </summary>
        /// <param name="request">The clan destruction request containing the master's ID.</param>
        /// <returns>A response indicating success or error details.</returns>
        [HttpPost("destroy")]
        public async Task<Response.DestroyClan> Destroy(Request.DestroyClan request)
        {
            await using var db = _dbContext.Connection(-1);
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            try
            {
                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(request.Master)))
                {
                    var ch = await _dbContext.Character.Get(request.Master) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var sync = await _dbContext.CharacterSync.Get(ch.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (sync.Clan == null)
                        throw new LogicException(ErrorCode.ClanNotJoined);

                    await using (await _distributedLock.Lock(Clan.DistributedLockKey(sync.Clan.Value)))
                    {
                        var clan = await _dbContext.Clan.Get(sync.Clan.Value) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        var members = (await _dbContext.ClanMember.Get(clan.Id)).ToList();
                        if (members.Count != 1)
                            throw new LogicException(ErrorCode.ClanMemberExists);

                        var master = members[0];
                        if (master.User != ch.Id)
                            throw new LogicException(ErrorCode.ClanNoPrivilege);

                        if (master.Role != (uint)ClanRole.Master)
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

        /// <summary>
        /// Handles clan title setting requests.
        /// Allows updating the clan's title with validation for length and uniqueness.
        /// The changer must have Master role or higher privileges to modify the clan title.
        /// </summary>
        /// <param name="request">The title setting request containing changer UID and new title.</param>
        /// <returns>A response confirming the title change or error details.</returns>
        [HttpPost("title")]
        public async Task<Response.SetClanTitle> SetTitle(Request.SetClanTitle request)
        {
            try
            {
                // Get changer character
                var changer = await _dbContext.Character.Get(request.Changer) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(changer.Id)))
                {
                    var sync = await _dbContext.CharacterSync.Get(changer.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (sync.Clan == null)
                        throw new LogicException(ErrorCode.ClanNotJoined);

                    await using (await _distributedLock.Lock(Clan.DistributedLockKey(sync.Clan.Value)))
                    {
                        var clan = await _dbContext.Clan.Get(sync.Clan.Value) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        // Get changer's clan member info
                        var changerMember = await _dbContext.ClanMember.Get(clan.Id, changer.Id) ??
                            throw new LogicException(ErrorCode.NotFoundClanMember);

                        // Check if changer has sufficient privileges (Master role or higher)
                        if (changerMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumChangeTitlePrivilege)
                            throw new LogicException(ErrorCode.ClanNoPrivilege);

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
            }
            catch (LogicException e)
            {
                return new Response.SetClanTitle
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.SetClanTitle
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }

        /// <summary>
        /// Handles clan join requests for new members.
        /// Adds a character to an existing clan with appropriate member position.
        /// Validates that the inviter has sufficient privileges to invite members.
        /// </summary>
        /// <param name="request">The join request containing inviter and invitee user IDs.</param>
        /// <returns>A response with the new member information or error details.</returns>
        [HttpPost("join")]
        public async Task<Response.JoinClan> Join(Request.JoinClan request)
        {
            try
            {
                // Get inviter character (the one doing the inviting)
                var inviter = await _dbContext.Character.Get(request.InviterUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Get invitee character (the one being invited)
                var invitee = await _dbContext.Character.Get(request.InviteeUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Prevent self-invitation
                if (inviter.Id == invitee.Id)
                    throw new LogicException(ErrorCode.CannotInviteSelf);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(inviter.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(invitee.Id)))
                    {
                        var inviterSync = await _dbContext.CharacterSync.Get(inviter.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var inviteeSync = await _dbContext.CharacterSync.Get(invitee.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Check if inviter is in a clan
                        if (inviterSync.Clan == null)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        // Check if invitee is already in a clan
                        if (inviteeSync.Clan != null)
                            throw new LogicException(ErrorCode.ClanAlreadyJoined);

                        await using (await _distributedLock.Lock(Clan.DistributedLockKey(inviterSync.Clan.Value)))
                        {
                            var clan = await _dbContext.Clan.Get(inviterSync.Clan.Value) ??
                                throw new LogicException(ErrorCode.NotFoundClan);

                            // Get inviter's clan member info
                            var inviterMember = await _dbContext.ClanMember.Get(clan.Id, inviter.Id) ??
                                throw new LogicException(ErrorCode.NotFoundClanMember);

                            // Check if inviter has sufficient privileges
                            if (inviterMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumInvitePrivilege)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Add invitee to clan
                            var cm = _dbContext.ClanMember.Set(new ClanMember
                            {
                                Clan = clan.Id,
                                Role = (uint)ClanRole.Mate,
                                User = invitee.Id,
                                Deleted = false
                            });

                            inviteeSync.Clan = clan.Id;
                            _dbContext.CharacterSync.Set(inviteeSync);

                            await _dbContext.SaveChangesAsync();
                            var response = new Response.JoinClan
                            {
                                Host = request.Host,
                                Clan = clan.Id,
                                Member = new Protocol.ClanMember
                                {
                                    Name = invitee.Name,
                                    Uid = cm.User,
                                    Role = cm.Role
                                },
                                Error = (uint)ErrorCode.None
                            };

                            _rabbitMqService.Publish(response, "amq.direct", $"fb.clan");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.JoinClan
                {
                    Host = request.Host,
                    Clan = 0,
                    Member = new Protocol.ClanMember(),
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.JoinClan
                {
                    Host = request.Host,
                    Clan = 0,
                    Member = new Protocol.ClanMember(),
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Handles voluntary clan leave requests for members.
        /// Allows members to leave the clan on their own initiative.
        /// </summary>
        /// <param name="request">The leave request containing member name and clan ID.</param>
        /// <returns>A response confirming the member removal or error details.</returns>
        [HttpPost("leave")]
        public async Task<Response.LeaveClan> Leave(Request.LeaveClan request)
        {
            try
            {
                var uid = await _dbContext.Character.GetCharacterId(request.Name) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(uid)))
                {
                    var ch = await _dbContext.Character.Get(uid) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var sync = await _dbContext.CharacterSync.Get(ch.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (sync.Clan == null)
                        throw new LogicException(ErrorCode.ClanNotJoined);

                    if (sync.Clan.Value != request.Clan)
                        throw new LogicException(ErrorCode.ClanNotMatched);

                    await using (await _distributedLock.Lock(Clan.DistributedLockKey(sync.Clan.Value)))
                    {
                        var clan = await _dbContext.Clan.Get(sync.Clan.Value) ??
                            throw new LogicException(ErrorCode.NotFoundClan);

                        var member = await _dbContext.ClanMember.Get(clan.Id, ch.Id) ??
                            throw new LogicException(ErrorCode.NotFoundClanMember);

                        if (member.Role == (uint)ClanRole.Master)
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



        /// <summary>
        /// Handles clan member expulsion by authorized members.
        /// Allows clan members with sufficient privileges to forcefully remove another member.
        /// </summary>
        /// <param name="request">The kick clan request containing host, clan, kicker and target information.</param>
        /// <returns>A response with the updated clan information and action taken.</returns>
        [HttpPost("kick")]
        public async Task<Response.KickClan> Kick(Request.KickClan request)
        {
            try
            {
                // Get kicker character
                var kickerUid = await _dbContext.Character.GetCharacterId(request.Kicker) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var kicker = await _dbContext.Character.Get(kickerUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Get target character
                var targetUid = await _dbContext.Character.GetCharacterId(request.Target) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var target = await _dbContext.Character.Get(targetUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Prevent self-kicking
                if (kicker.Name == target.Name)
                    throw new LogicException(ErrorCode.CannotKickSelf);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(kicker.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var kickerSync = await _dbContext.CharacterSync.Get(kicker.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Check if kicker is in a clan
                        if (kickerSync.Clan == null)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        // Check if target is in the same clan
                        if (targetSync.Clan != kickerSync.Clan)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        await using (await _distributedLock.Lock(Clan.DistributedLockKey(kickerSync.Clan.Value)))
                        {
                            var clan = await _dbContext.Clan.Get(kickerSync.Clan.Value) ??
                                throw new LogicException(ErrorCode.NotFoundClan);

                            // Get kicker's clan member info
                            var kickerMember = await _dbContext.ClanMember.Get(clan.Id, kicker.Id) ??
                                throw new LogicException(ErrorCode.NotFoundClanMember);

                            // Get target's clan member info
                            var targetMember = await _dbContext.ClanMember.Get(clan.Id, target.Id) ??
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
                            _dbContext.ClanMember.Set(targetMember);

                            targetSync.Clan = null;
                            _dbContext.CharacterSync.Set(targetSync);

                            var response = new Response.KickClan
                            {
                                Host = request.Host,
                                Clan = request.Clan,
                                Uid = target.Id,
                                Uname = target.Name,
                                Error = 0
                            };

                            await _dbContext.SaveChangesAsync();
                            _rabbitMqService.Publish(response, "amq.direct", $"fb.clan");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.KickClan
                {
                    Host = request.Host,
                    Clan = request.Clan,
                    Uid = 0,
                    Uname = request.Target,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.KickClan
                {
                    Host = request.Host,
                    Clan = request.Clan,
                    Uid = 0,
                    Uname = request.Target,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Handles clan broadcast message requests.
        /// Sends messages to all members of a specific clan via RabbitMQ.
        /// </summary>
        /// <param name="request">The clan broadcast request containing clan ID and message information.</param>
        /// <returns>A response confirming the broadcast was sent or error details.</returns>
        [HttpPost("broadcast")]
        public async Task<Response.BroadcastClan> Broadcast(Request.BroadcastClan request)
        {
            try
            {
                await using (await _distributedLock.Lock(Clan.DistributedLockKey(request.Clan)))
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

        /// <summary>
        /// Handles clan member position change requests.
        /// Allows authorized clan members to change the position of other members within the clan hierarchy.
        /// </summary>
        /// <param name="request">The position change request containing changer ID, target name, clan ID, and new position.</param>
        /// <returns>A response confirming the position change or error details.</returns>
        [HttpPost("change-role")]
        public async Task<Response.ChangeClanRole> ChangeRole(Request.ChangeClanRole request)
        {
            try
            {
                // Get changer character (the one making the change)
                var changer = await _dbContext.Character.Get(request.ChangerUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Get target character (the one being changed)
                var targetUid = await _dbContext.Character.GetCharacterId(request.TargetName) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var target = await _dbContext.Character.Get(targetUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Prevent self-role change
                if (changer.Id == target.Id)
                    throw new LogicException(ErrorCode.CannotChangeClanRoleSelf);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(changer.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var changerSync = await _dbContext.CharacterSync.Get(changer.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Check if changer is in a clan
                        if (changerSync.Clan == null)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        // Check if target is in the same clan
                        if (targetSync.Clan != changerSync.Clan)
                            throw new LogicException(ErrorCode.ClanNotJoined);

                        await using (await _distributedLock.Lock(Clan.DistributedLockKey(changerSync.Clan.Value)))
                        {
                            var clan = await _dbContext.Clan.Get(changerSync.Clan.Value) ??
                                throw new LogicException(ErrorCode.NotFoundClan);

                            // Get changer's clan member info
                            var changerMember = await _dbContext.ClanMember.Get(clan.Id, changer.Id) ??
                                throw new LogicException(ErrorCode.NotFoundClanMember);

                            // Get target's clan member info
                            var targetMember = await _dbContext.ClanMember.Get(clan.Id, target.Id) ??
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
                            _dbContext.ClanMember.Set(targetMember);

                            var response = new Response.ChangeClanRole
                            {
                                Host = request.Host,
                                Clan = clan.Id,
                                ChangerUid = changer.Id,
                                ChangerName = changer.Name,
                                TargetUid = target.Id,
                                TargetName = target.Name,
                                OldRole = oldRole,
                                NewRole = request.NewRole,
                                Error = (uint)ErrorCode.None
                            };

                            await _dbContext.SaveChangesAsync();
                            _rabbitMqService.Publish(response, "amq.direct", $"fb.clan");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.ChangeClanRole
                {
                    Host = request.Host,
                    Clan = request.Clan,
                    ChangerUid = request.ChangerUid,
                    ChangerName = "",
                    TargetUid = 0,
                    TargetName = request.TargetName,
                    OldRole = 0,
                    NewRole = request.NewRole,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.ChangeClanRole
                {
                    Host = request.Host,
                    Clan = request.Clan,
                    ChangerUid = request.ChangerUid,
                    ChangerName = "",
                    TargetUid = 0,
                    TargetName = request.TargetName,
                    OldRole = 0,
                    NewRole = request.NewRole,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}
