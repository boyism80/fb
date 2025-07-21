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
                    Position = x.Position
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

        /// <summary>
        /// Handles clan title setting requests.
        /// Allows updating the clan's title with validation for length and uniqueness.
        /// </summary>
        /// <param name="request">The title setting request containing clan ID and new title.</param>
        /// <returns>A response confirming the title change or error details.</returns>
        [HttpPost("title")]
        public async Task<Response.SetClanTitle> SetTitle(Request.SetClanTitle request)
        {
            try
            {
                await using (await _distributedLock.Lock(Clan.DistributedLockKey(request.Clan)))
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

        /// <summary>
        /// Handles clan join requests for new members.
        /// Adds a character to an existing clan with appropriate member position.
        /// </summary>
        /// <param name="request">The join request containing user ID and target clan ID.</param>
        /// <returns>A response with the new member information or error details.</returns>
        [HttpPost("join")]
        public async Task<Response.JoinClan> Join(Request.JoinClan request)
        {
            try
            {
                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(request.Uid)))
                {
                    var target = await _dbContext.Character.Get(request.Uid) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (targetSync.Clan != null)
                        throw new LogicException(ErrorCode.ClanAlreadyJoined);

                    await using (await _distributedLock.Lock(Clan.DistributedLockKey(request.Clan)))
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
                    throw new LogicException(ErrorCode.CannotGroupSelf);

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

                            // Check if kicker has minimum kickable position (Mate or higher)
                            if (kickerMember.Position < (uint)ClanPosition.Mate)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Check if target has higher or equal position (can't kick superiors)
                            if (targetMember.Position >= kickerMember.Position)
                                throw new LogicException(ErrorCode.ClanNoPrivilege);

                            // Check if target is not the master (can't kick master)
                            if (targetMember.Position == (uint)ClanPosition.Master)
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
    }
}
