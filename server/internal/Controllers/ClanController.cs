using AutoMapper;
using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Extension;
using Http.Model;
using Http.Service;
using Medallion.Threading.Redis;
using Microsoft.AspNetCore.Mvc;
using StackExchange.Redis;
using Protocol = fb.protocol.@internal;
using Request = fb.protocol.@internal.request;
using Response = fb.protocol.@internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("clan")]
    public class ClanController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly IMapper _mapper;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly SessionService _sessionService;
        private readonly ILogger<ClanController> _logger;
        public ClanController(IConfiguration configuration,
            DbContext dbContext,
            RabbitMqService rabbitMqService,
            IMapper mapper,
            RedisDistributedLockService distributedLock,
            SessionService sessionService,
            ILogger<ClanController> logger)
        {
            _configuration = configuration;
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _mapper = mapper;
            _distributedLock = distributedLock;
            _sessionService = sessionService;
            _logger = logger;
        }

        private async Task<List<Protocol.ClanMember>> GetClanMemberResponse(uint world, uint id)
        {
            var members = await _dbContext.ClanMember.Get(world, id);
            await using var conn = _dbContext.GetUnifiedConnection();
            var names = await conn.QueryAsync($"SELECT `id`, `name` FROM `name_registry` WHERE id IN ({string.Join(',', members.Select(x => x.User))})");
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

        private void Touch(uint world, params Clan[] clans)
        {
            var at = DateTime.UtcNow;
            foreach (var clan in clans)
            {
                clan.UpdatedDate = at;
                clan.Revision++;
                _dbContext.Clan.Set(world, clan);
            }
        }

        private sealed class ClanLocks : IAsyncDisposable
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

        // Every path that touches more than one clan locks them in id order so two such paths cannot deadlock.
        private async Task<ClanLocks> LockClans(uint world, IEnumerable<uint> ids)
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

        private async Task<Protocol.Clan> GetClanResponse(uint world, Clan clan)
        {
            var protocolClan = _mapper.Map<Protocol.Clan>(clan);
            protocolClan.Revision = clan.Revision;
            var alliance = await _dbContext.ClanAlliance.Get(world, clan.Id);
            protocolClan.AlliedClanId = alliance?.AlliedClan;
            var enemies = await _dbContext.ClanEnemy.Get(world, clan.Id);
            protocolClan.EnemyClanIds = enemies.Select(x => x.EnemyClan).ToList();
            return protocolClan;
        }

        [HttpGet("id/{id}")]
        public async Task<Response.ClanDetails> GetById(uint id)
        {
            try
            {
                var world = await _dbContext.Clan.GetWorld(id);
                if (!world.HasValue)
                {
                    throw new LogicException(ErrorCode.NotFoundClan);
                }

                return await Get(world.Value, id);
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
        }

        [HttpGet("{world}/{id}")]
        public async Task<Response.ClanDetails> Get(uint world, uint id)
        {
            try
            {
                await using var _ = await _distributedLock.Lock(world, Clan.DistributedLockKey(id));

                var clan = await _dbContext.Clan.Get(world, id) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                return new Response.ClanDetails
                {
                    Action = Protocol.ClanDetailsAction.Query,
                    Clan = await GetClanResponse(world, clan),
                    Members = await GetClanMemberResponse(world, id),
                    Revision = clan.Revision,
                    Error = (uint)ErrorCode.None
                };
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
            var world = request.World;
            await using var db = _dbContext.GetUnifiedConnection();
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            var committed = false;
            try
            {
                var ch = await _dbContext.Character.Get(world, request.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var existing = await db.QueryFirstOrDefaultAsync<uint?>(
                    @"SELECT id FROM clan_name WHERE name = @Name FOR UPDATE",
                    new { Name = request.Name },
                    transaction: trans
                );
                if (existing.HasValue)
                {
                    throw new LogicException(ErrorCode.ClanNameAlreadyExists);
                }

                var newClanId = await db.QuerySingleAsync<uint>(
                    @"INSERT INTO clan_name (name, world, created_date, updated_date) VALUES (@Name, @World, UTC_TIMESTAMP(3), UTC_TIMESTAMP(3));SELECT LAST_INSERT_ID();",
                    new { Name = request.Name, World = world },
                    transaction: trans
                );

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(ch.Id));
                await using var _2 = await _distributedLock.Lock(world, Clan.DistributedLockKey(newClanId));

                var sync = await _dbContext.CharacterRealtimeState.Get(world, ch.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                await _dbContext.CharacterRealtimeState.ClearStaleMembership(world, sync);
                if (sync.Clan != null)
                {
                    throw new LogicException(ErrorCode.ClanAlreadyJoined);
                }

                var clan = _dbContext.Clan.Set(world, new Clan
                {
                    Id = newClanId,
                    Name = request.Name,
                    Title = null,
                    Revision = 1,
                });
                await _dbContext.Clan.SaveChangesAsync();

                sync.Clan = clan.Id;
                _dbContext.CharacterRealtimeState.Set(world, sync);
                await _dbContext.CharacterRealtimeState.SaveChangesAsync();

                _dbContext.ClanMember.Set(world, new ClanMember
                {
                    Clan = clan.Id,
                    User = request.Master,
                    Role = (uint)ClanRole.Master
                });
                await _dbContext.ClanMember.SaveChangesAsync();
                Touch(world, clan);
                await _dbContext.Clan.SaveChangesAsync();
                await trans.CommitAsync();
                committed = true;

                var response = new Response.ClanDetails
                {
                    Host = request.Host,
                    Action = Protocol.ClanDetailsAction.Create,
                    Clan = await GetClanResponse(world, clan),
                    Members = await GetClanMemberResponse(world, clan.Id),
                    Revision = clan.Revision,
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
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
                if (committed == false)
                {
                    await trans.RollbackAsync();
                }

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
            var world = request.World;
            await using var db = _dbContext.GetUnifiedConnection();
            await db.OpenAsync();
            await using var trans = await db.BeginTransactionAsync();
            var committed = false;
            var castleLocks = new List<RedisDistributedLockHandle>();
            try
            {
                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(request.Master));

                var ch = await _dbContext.Character.Get(world, request.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var sync = await _dbContext.CharacterRealtimeState.Get(world, ch.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                if (sync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                var counterpartIds = (await _dbContext.ClanEnemy.Get(world, sync.Clan.Value)).Select(x => x.EnemyClan).ToList();
                var alliedBefore = await _dbContext.ClanAlliance.Get(world, sync.Clan.Value);
                if (alliedBefore != null)
                {
                    counterpartIds.Add(alliedBefore.AlliedClan);
                }

                await using var _2 = await LockClans(world, counterpartIds.Append(sync.Clan.Value));

                var clan = await _dbContext.Clan.Get(world, sync.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                var members = (await _dbContext.ClanMember.Get(world, clan.Id)).ToList();
                if (members.Count != 1)
                {
                    throw new LogicException(ErrorCode.ClanMemberExists);
                }

                var master = members[0];
                if (master.User != ch.Id)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                if (master.Role != (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                _dbContext.ClanMember.Delete(world, master);

                var oldTitle = clan.Title;
                _dbContext.Clan.Delete(world, clan.Id);

                var alliance = await _dbContext.ClanAlliance.Get(world, clan.Id);
                await using var _3 = alliance != null ? await _distributedLock.Lock(world, ClanAlliance.DistributedLockKey(alliance.AlliedClan)) : null;
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
                        _dbContext.Castle.Set(world, new Castle { DivineBeast = divineBeast, OwnerClan = null });
                        clearedCastles.Add(divineBeast);
                    }
                }

                await db.ExecuteAsync("USP_CLAN_NAME_DELETE", new
                {
                    id = clan.Id
                }, transaction: trans, commandType: System.Data.CommandType.StoredProcedure);

                var archived = clan.MoneyRequests
                    .Select(x => new ClanMoneyRequestArchive { Clan = clan.Id, RequestId = x.Id, Applied = x.Applied })
                    .ToArray();
                if (archived.Length > 0)
                {
                    _dbContext.ClanMoneyRequestArchive.Set(world, archived);
                    await _dbContext.ClanMoneyRequestArchive.SaveChangesAsync();
                }

                await _dbContext.ClanMember.SaveChangesAsync();
                await _dbContext.SaveChangesAsync();

                sync.Clan = null;
                _dbContext.CharacterRealtimeState.Set(world, sync);
                await _dbContext.CharacterRealtimeState.SaveChangesAsync();
                await trans.CommitAsync();
                committed = true;

                var conn = _dbContext.GetUnifiedConnection();
                var masterName = await conn.QueryFirstOrDefaultAsync<string>(
                    $"SELECT `name` FROM `name_registry` WHERE id = {ch.Id}");

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
                    Revision = clan.Revision + 1,
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);

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
                        Actor = response.Actor,
                        RelatedClanId = clan.Id,
                        Error = (uint)ErrorCode.None
                    }, "clan", request.World);
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
                        Actor = response.Actor,
                        RelatedClanId = clan.Id,
                        Error = (uint)ErrorCode.None
                    }, "clan", request.World);
                }

                foreach (var divineBeast in clearedCastles)
                {
                    await _rabbitMqService.NotifySavedAsync(new Response.UpdatedCastle
                    {
                        Host = request.Host,
                        DivineBeast = divineBeast,
                        OwnerClanId = null,
                        Error = (uint)ErrorCode.None
                    }, AmqpRoute.Exchange, AmqpRoute.Home("castle", request.World));
                }
                return response;
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
                if (committed == false)
                {
                    await trans.RollbackAsync();
                }

                return new Response.DestroyClan
                {
                    Host = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            {
                foreach (var handle in castleLocks)
                {
                    await handle.DisposeAsync();
                }

                await db.CloseAsync();
            }
        }
        [HttpPost("title")]
        public async Task<Response.UpdatedClan> SetTitle(Request.SetClanTitle request)
        {
            try
            {
                var world = request.World;
                // Get changer character
                var changer = await _dbContext.Character.Get(world, request.Changer) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(changer.Id));

                var sync = await _dbContext.CharacterRealtimeState.Get(world, changer.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                if (sync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                await using var _2 = await _distributedLock.Lock(world, Clan.DistributedLockKey(sync.Clan.Value));

                var clan = await _dbContext.Clan.Get(world, sync.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                // Get changer's clan member info
                var changerMember = await _dbContext.ClanMember.Get(world, clan.Id, changer.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                // Check if changer has sufficient privileges (Master role or higher)
                if (changerMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumChangeTitlePrivilege)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                if (clan.Title == request.Title)
                {
                    throw new LogicException(ErrorCode.ClanTitleNotChanged);
                }

                if (request.Title != null && request.Title.Length < 2)
                {
                    throw new LogicException(ErrorCode.ClanTitleTooShort);
                }

                var oldTitle = clan.Title;
                clan.Title = request.Title;
                Touch(world, clan);

                await _dbContext.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.SetTitle,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
                    Actor = new Protocol.CharacterRef
                    {
                        Uid = changer.Id,
                        Name = changer.Name
                    },
                    OldTitle = oldTitle,
                    NewTitle = request.Title,
                    Error = (uint)ErrorCode.None
                };
                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
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
                var world = request.World;
                // Get inviter character (the one doing the inviting)
                var inviter = await _dbContext.Character.Get(world, request.InviterUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (inviter.Name == request.InviteeName)
                {
                    throw new LogicException(ErrorCode.CannotInviteSelf);
                }

                // Get invitee session and character (the one being invited)
                var inviteeSession = await _sessionService.Get(world, request.InviteeName) ??
                    throw new LogicException(ErrorCode.Offline);

                var invitee = await _dbContext.Character.Get(world, inviteeSession.Uid) ??
                    throw new LogicException(ErrorCode.Offline);

                // Prevent self-invitation
                if (inviter.Id == invitee.Id)
                {
                    throw new LogicException(ErrorCode.CannotInviteSelf);
                }

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(Math.Min(inviter.Id, invitee.Id)));
                await using var _2 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(Math.Max(inviter.Id, invitee.Id)));

                var inviterSync = await _dbContext.CharacterRealtimeState.Get(world, inviter.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                var inviteeSync = await _dbContext.CharacterRealtimeState.Get(world, invitee.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                // Check if inviter is in a clan
                if (inviterSync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                await _dbContext.CharacterRealtimeState.ClearStaleMembership(world, inviteeSync);
                if (inviteeSync.Clan != null)
                {
                    throw new LogicException(ErrorCode.ClanAlreadyJoined);
                }

                await using var _3 = await _distributedLock.Lock(world, Clan.DistributedLockKey(inviterSync.Clan.Value));

                var clan = await _dbContext.Clan.Get(world, inviterSync.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                // Get inviter's clan member info
                var inviterMember = await _dbContext.ClanMember.Get(world, clan.Id, inviter.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                // Check if inviter has sufficient privileges
                if (inviterMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumInvitePrivilege)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                var members = await _dbContext.ClanMember.Get(world, clan.Id);
                if (members.Count() >= Fb.Model.ConstValue.Clan.MaxMember)
                {
                    throw new LogicException(ErrorCode.ClanFullMember);
                }

                inviteeSync.Clan = clan.Id;
                _dbContext.CharacterRealtimeState.Set(world, inviteeSync);
                await _dbContext.CharacterRealtimeState.SaveChangesAsync();

                Touch(world, clan);
                _dbContext.ClanMember.Set(world, new ClanMember
                {
                    Clan = clan.Id,
                    Role = (uint)ClanRole.Mate,
                    User = invitee.Id,
                });
                await _dbContext.ClanMember.SaveChangesAsync();
                await _dbContext.Clan.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Join,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
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

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
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
                var world = request.World;
                var uid = await _dbContext.Character.GetCharacterId(world, request.Name) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(uid));

                var ch = await _dbContext.Character.Get(world, uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var sync = await _dbContext.CharacterRealtimeState.Get(world, ch.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                if (sync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                if (sync.Clan.Value != request.Clan)
                {
                    throw new LogicException(ErrorCode.ClanNotMatched);
                }

                await using var _2 = await _distributedLock.Lock(world, Clan.DistributedLockKey(sync.Clan.Value));

                var clan = await _dbContext.Clan.Get(world, sync.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                var member = await _dbContext.ClanMember.Get(world, clan.Id, ch.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (member.Role == (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.ClanCannotLeaveeMaster);
                }

                Touch(world, clan);
                _dbContext.ClanMember.Delete(world, member);
                await _dbContext.ClanMember.SaveChangesAsync();
                await _dbContext.Clan.SaveChangesAsync();

                sync.Clan = null;
                _dbContext.CharacterRealtimeState.Set(world, sync);
                await _dbContext.CharacterRealtimeState.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Leave,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
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

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
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
                var world = request.World;
                // Get kicker character
                var kickerUid = await _dbContext.Character.GetCharacterId(world, request.Kicker) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var kicker = await _dbContext.Character.Get(world, kickerUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Get target character
                var targetUid = await _dbContext.Character.GetCharacterId(world, request.Target) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var target = await _dbContext.Character.Get(world, targetUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Prevent self-kicking
                if (kicker.Name == target.Name)
                {
                    throw new LogicException(ErrorCode.CannotKickSelf);
                }

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(Math.Min(kicker.Id, target.Id)));
                await using var _2 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(Math.Max(kicker.Id, target.Id)));

                var kickerSync = await _dbContext.CharacterRealtimeState.Get(world, kicker.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                var targetSync = await _dbContext.CharacterRealtimeState.Get(world, target.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                // Check if kicker is in a clan
                if (kickerSync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                // Check if target is in the same clan
                if (targetSync.Clan != kickerSync.Clan)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                await using var _3 = await _distributedLock.Lock(world, Clan.DistributedLockKey(kickerSync.Clan.Value));

                var clan = await _dbContext.Clan.Get(world, kickerSync.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                // Get kicker's clan member info
                var kickerMember = await _dbContext.ClanMember.Get(world, clan.Id, kicker.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                // Get target's clan member info
                var targetMember = await _dbContext.ClanMember.Get(world, clan.Id, target.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                // Check if kicker has minimum kickable role
                if (kickerMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumKickPrivilege)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                // Check if target has higher or equal role (can't kick superiors)
                if (targetMember.Role >= kickerMember.Role)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                // Check if target is not the master (can't kick master)
                if (targetMember.Role == (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.ClanCannotLeaveeMaster);
                }

                Touch(world, clan);
                _dbContext.ClanMember.Delete(world, targetMember);
                await _dbContext.ClanMember.SaveChangesAsync();
                await _dbContext.Clan.SaveChangesAsync();

                targetSync.Clan = null;
                _dbContext.CharacterRealtimeState.Set(world, targetSync);
                await _dbContext.CharacterRealtimeState.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Kick,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
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

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
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
                var world = await _dbContext.Clan.GetWorld(request.Clan) ?? request.World;
                await using var _ = await _distributedLock.Lock(world, Clan.DistributedLockKey(request.Clan));

                var clan = await _dbContext.Clan.Get(world, request.Clan) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                var response = new Response.BroadcastClan
                {
                    Host = request.Host,
                    Clan = request.Clan,
                    Message = request.Message,
                    Type = request.Type,
                    Error = (uint)ErrorCode.None
                };
                await _rabbitMqService.PublishFanoutAsync(response, "clan", world);
                return response;
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
                var world = request.World;
                // Get changer character (the one making the change)
                var changer = await _dbContext.Character.Get(world, request.ChangerUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Get target character (the one being changed)
                var targetUid = await _dbContext.Character.GetCharacterId(world, request.TargetName) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var target = await _dbContext.Character.Get(world, targetUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Prevent self-role change
                if (changer.Id == target.Id)
                {
                    throw new LogicException(ErrorCode.CannotChangeClanRoleSelf);
                }

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(Math.Min(changer.Id, target.Id)));
                await using var _2 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(Math.Max(changer.Id, target.Id)));

                var changerSync = await _dbContext.CharacterRealtimeState.Get(world, changer.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                var targetSync = await _dbContext.CharacterRealtimeState.Get(world, target.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                // Check if changer is in a clan
                if (changerSync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                // Check if target is in the same clan
                if (targetSync.Clan != changerSync.Clan)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                await using var _3 = await _distributedLock.Lock(world, Clan.DistributedLockKey(changerSync.Clan.Value));

                var clan = await _dbContext.Clan.Get(world, changerSync.Clan.Value) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                // Get changer's clan member info
                var changerMember = await _dbContext.ClanMember.Get(world, clan.Id, changer.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                // Get target's clan member info
                var targetMember = await _dbContext.ClanMember.Get(world, clan.Id, target.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                // Check if changer has sufficient privileges
                if (changerMember.Role < (uint)Fb.Model.ConstValue.Clan.MinimumChangeRolePrivilege)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                // Check if target has higher or equal role (can't change superiors)
                if (targetMember.Role >= changerMember.Role)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                // Check if trying to change master role (only master can change master)
                if (targetMember.Role == (uint)ClanRole.Master &&
                    changerMember.Role != (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                // Validate new role
                if (request.NewRole < (uint)ClanRole.Mate ||
                    request.NewRole > (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.InvalidClanRole);
                }

                // Check if new role is higher than changer's role (can't promote to higher rank)
                if (request.NewRole >= changerMember.Role)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                // Store old role for response
                var oldRole = targetMember.Role;

                // Update target's role
                targetMember.Role = request.NewRole;
                Touch(world, clan);
                _dbContext.ClanMember.Set(world, targetMember);

                await _dbContext.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.ChangeRole,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
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

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
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
        [HttpPost("ally")]
        public async Task<Response.UpdatedClan> Ally(Request.AllyClan request)
        {
            try
            {
                var world = request.World;
                var requester = await _dbContext.Character.Get(world, request.RequesterUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(requester.Id));

                var requesterSync = await _dbContext.CharacterRealtimeState.Get(world, requester.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                if (requesterSync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                if (requesterSync.Clan.Value != request.Clan)
                {
                    throw new LogicException(ErrorCode.ClanNotMatched);
                }

                if (request.Clan == request.TargetClan)
                {
                    throw new LogicException(ErrorCode.ClanCannotAllySelf);
                }

                await using var _2 = await LockClans(world, new[] { request.Clan, request.TargetClan });

                var clan = await _dbContext.Clan.Get(world, request.Clan) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                var requesterMember = await _dbContext.ClanMember.Get(world, clan.Id, requester.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (requesterMember.Role != (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                var targetClan = await _dbContext.Clan.Get(world, request.TargetClan) ??
                    throw new LogicException(ErrorCode.ClanTargetNotFound);

                await using var _3 = await _distributedLock.Lock(world, ClanAlliance.DistributedLockKey(Math.Min(request.Clan, request.TargetClan)));
                await using var _4 = await _distributedLock.Lock(world, ClanAlliance.DistributedLockKey(Math.Max(request.Clan, request.TargetClan)));

                if (await _dbContext.ClanAlliance.Get(world, request.Clan) != null)
                {
                    throw new LogicException(ErrorCode.ClanAlreadyAllied);
                }

                if (await _dbContext.ClanAlliance.Get(world, request.TargetClan) != null)
                {
                    throw new LogicException(ErrorCode.ClanAlreadyAllied);
                }

                if (await _dbContext.ClanEnemy.Get(world, request.Clan, request.TargetClan) != null)
                {
                    throw new LogicException(ErrorCode.ClanCannotAllyEnemy);
                }

                _dbContext.ClanAlliance.Set(world, new ClanAlliance { Clan = request.Clan, AlliedClan = request.TargetClan });
                _dbContext.ClanAlliance.Set(world, new ClanAlliance { Clan = request.TargetClan, AlliedClan = request.Clan });
                Touch(world, clan, targetClan);

                await _dbContext.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Ally,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
                    RelatedRevision = targetClan.Revision,
                    Actor = new Protocol.CharacterRef
                    {
                        Uid = requester.Id,
                        Name = requester.Name
                    },
                    RelatedClanId = targetClan.Id,
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Ally,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Ally,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("unally")]
        public async Task<Response.UpdatedClan> Unally(Request.UnallyClan request)
        {
            try
            {
                var world = request.World;
                var requester = await _dbContext.Character.Get(world, request.RequesterUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(requester.Id));

                var requesterSync = await _dbContext.CharacterRealtimeState.Get(world, requester.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                if (requesterSync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                if (requesterSync.Clan.Value != request.Clan)
                {
                    throw new LogicException(ErrorCode.ClanNotMatched);
                }

                var alliedBefore = await _dbContext.ClanAlliance.Get(world, request.Clan) ??
                    throw new LogicException(ErrorCode.ClanNotAllied);

                await using var _2 = await LockClans(world, new[] { request.Clan, alliedBefore.AlliedClan });

                var clan = await _dbContext.Clan.Get(world, request.Clan) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                var requesterMember = await _dbContext.ClanMember.Get(world, clan.Id, requester.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (requesterMember.Role != (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                await using var _3 = await _distributedLock.Lock(world, ClanAlliance.DistributedLockKey(request.Clan));

                var alliance = await _dbContext.ClanAlliance.Get(world, request.Clan) ??
                    throw new LogicException(ErrorCode.ClanNotAllied);

                if (alliance.AlliedClan != alliedBefore.AlliedClan)
                {
                    throw new LogicException(ErrorCode.Unhandled);
                }

                var alliedClanId = alliance.AlliedClan;
                await using var _4 = await _distributedLock.Lock(world, ClanAlliance.DistributedLockKey(alliedClanId));

                _dbContext.ClanAlliance.Delete(world, request.Clan);
                _dbContext.ClanAlliance.Delete(world, alliedClanId);
                var alliedClan = await _dbContext.Clan.Get(world, alliedClanId);
                if (alliedClan != null)
                {
                    Touch(world, clan, alliedClan);
                }
                else
                {
                    Touch(world, clan);
                }

                await _dbContext.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Unally,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
                    RelatedRevision = alliedClan?.Revision ?? 0,
                    Actor = new Protocol.CharacterRef
                    {
                        Uid = requester.Id,
                        Name = requester.Name
                    },
                    RelatedClanId = alliedClanId,
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Unally,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Unally,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("enemy")]
        public async Task<Response.UpdatedClan> Enemy(Request.DeclareClanEnemy request)
        {
            try
            {
                var world = request.World;
                var requester = await _dbContext.Character.Get(world, request.RequesterUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(requester.Id));

                var requesterSync = await _dbContext.CharacterRealtimeState.Get(world, requester.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                if (requesterSync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                if (requesterSync.Clan.Value != request.Clan)
                {
                    throw new LogicException(ErrorCode.ClanNotMatched);
                }

                if (request.Clan == request.TargetClan)
                {
                    throw new LogicException(ErrorCode.ClanCannotEnemySelf);
                }

                await using var _2 = await LockClans(world, new[] { request.Clan, request.TargetClan });

                var clan = await _dbContext.Clan.Get(world, request.Clan) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                var requesterMember = await _dbContext.ClanMember.Get(world, clan.Id, requester.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (requesterMember.Role != (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                var targetClan = await _dbContext.Clan.Get(world, request.TargetClan) ??
                    throw new LogicException(ErrorCode.ClanTargetNotFound);

                var alliance = await _dbContext.ClanAlliance.Get(world, request.Clan);
                if (alliance != null && alliance.AlliedClan == request.TargetClan)
                {
                    throw new LogicException(ErrorCode.ClanCannotEnemyAlly);
                }

                if (await _dbContext.ClanEnemy.Get(world, request.Clan, request.TargetClan) != null)
                {
                    throw new LogicException(ErrorCode.ClanAlreadyEnemy);
                }

                _dbContext.ClanEnemy.Set(world, new ClanEnemy { Clan = request.Clan, EnemyClan = request.TargetClan });
                _dbContext.ClanEnemy.Set(world, new ClanEnemy { Clan = request.TargetClan, EnemyClan = request.Clan });
                Touch(world, clan, targetClan);

                await _dbContext.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Enemy,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
                    RelatedRevision = targetClan.Revision,
                    Actor = new Protocol.CharacterRef
                    {
                        Uid = requester.Id,
                        Name = requester.Name
                    },
                    RelatedClanId = targetClan.Id,
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Enemy,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Enemy,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("money")]
        public async Task<Response.UpdatedClan> SetMoney(Request.SetClanMoney request)
        {
            try
            {
                var world = await _dbContext.Clan.GetWorld(request.Clan) ?? request.World;
                await using var _ = await _distributedLock.Lock(world, Clan.DistributedLockKey(request.Clan));

                var clan = await _dbContext.Clan.Get(world, request.Clan) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                if (request.RequestId == 0)
                {
                    throw new LogicException(ErrorCode.Unhandled);
                }

                var previous = await FindMoneyRequest(world, clan, request.RequestId);
                if (previous.HasValue)
                {
                    if (previous.Value == false)
                    {
                        throw new LogicException(ErrorCode.Unhandled);
                    }

                    return new Response.UpdatedClan
                    {
                        Host = request.Host,
                        Action = Protocol.ClanActionType.SetMoney,
                        ClanId = clan.Id,
                        ClanName = clan.Name,
                        Revision = clan.Revision,
                        Money = clan.Money,
                        Error = (uint)ErrorCode.None
                    };
                }

                if (request.Delta > 0)
                {
                    var next = clan.Money + (ulong)request.Delta;
                    if (next < clan.Money)
                    {
                        throw new LogicException(ErrorCode.Unhandled);
                    }

                    clan.Money = next;
                }
                else if (request.Delta < 0)
                {
                    var abs = (ulong)(-request.Delta);
                    if (clan.Money < abs)
                    {
                        throw new LogicException(ErrorCode.Unhandled);
                    }

                    clan.Money -= abs;
                }

                await ArchiveMoneyRequests(world, clan, DateTime.UtcNow);
                clan.MoneyRequests.Add(new ClanMoneyRequest { Id = request.RequestId, Applied = true, Date = DateTime.UtcNow });
                Touch(world, clan);
                await _dbContext.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.SetMoney,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
                    Money = clan.Money,
                    Error = (uint)ErrorCode.None
                };
                await _rabbitMqService.NotifySavedAsync(response, "clan", world);
                return response;
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.SetMoney,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.SetMoney,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("money/abort")]
        public async Task<Response.AbortClanMoney> AbortMoney(Request.AbortClanMoney request)
        {
            try
            {
                var world = await _dbContext.Clan.GetWorld(request.Clan) ?? request.World;
                await using var _ = await _distributedLock.Lock(world, Clan.DistributedLockKey(request.Clan));

                var clan = await _dbContext.Clan.Get(world, request.Clan);
                if (clan == null)
                {
                    var archived = await _dbContext.ClanMoneyRequestArchive.Get(world, request.Clan, request.RequestId);
                    return new Response.AbortClanMoney
                    {
                        Applied = archived?.Applied ?? false,
                        Error = (uint)ErrorCode.None
                    };
                }

                var previous = await FindMoneyRequest(world, clan, request.RequestId);
                if (previous == true)
                {
                    return new Response.AbortClanMoney
                    {
                        Applied = true,
                        Money = clan.Money,
                        Revision = clan.Revision,
                        Error = (uint)ErrorCode.None
                    };
                }

                if (previous == null)
                {
                    await ArchiveMoneyRequests(world, clan, DateTime.UtcNow);
                    clan.MoneyRequests.Add(new ClanMoneyRequest { Id = request.RequestId, Applied = false, Date = DateTime.UtcNow });
                    Touch(world, clan);
                    await _dbContext.SaveChangesAsync();
                }
                return new Response.AbortClanMoney
                {
                    Applied = false,
                    Revision = clan.Revision,
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                return new Response.AbortClanMoney
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.AbortClanMoney
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        // The caller must hold the clan lock.
        private async Task<bool?> FindMoneyRequest(uint world, Clan clan, ulong requestId)
        {
            var recent = clan.MoneyRequests.Find(x => x.Id == requestId);
            if (recent != null)
            {
                return recent.Applied;
            }

            var archived = await _dbContext.ClanMoneyRequestArchive.Get(world, clan.Id, requestId);
            return archived?.Applied;
        }

        // The caller must hold the clan lock. The archive is saved before the clan record drops the requests, so a
        // failure in between leaves them in both places instead of losing them.
        private async Task ArchiveMoneyRequests(uint world, Clan clan, DateTime now)
        {
            var expired = clan.MoneyRequests.Where(x => now - x.Date > Clan.MoneyRequestRetention).ToList();
            if (expired.Count == 0)
            {
                return;
            }

            _dbContext.ClanMoneyRequestArchive.Set(world, expired.Select(x => new ClanMoneyRequestArchive { Clan = clan.Id, RequestId = x.Id, Applied = x.Applied }).ToArray());
            await _dbContext.ClanMoneyRequestArchive.SaveChangesAsync();
            clan.MoneyRequests.RemoveAll(expired.Contains);
        }

        [HttpPost("unenemy")]
        public async Task<Response.UpdatedClan> Unenemy(Request.EndClanEnemy request)
        {
            try
            {
                var world = request.World;
                var requester = await _dbContext.Character.Get(world, request.RequesterUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using var _1 = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(requester.Id));

                var requesterSync = await _dbContext.CharacterRealtimeState.Get(world, requester.Id) ??
                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                if (requesterSync.Clan == null)
                {
                    throw new LogicException(ErrorCode.ClanNotJoined);
                }

                if (requesterSync.Clan.Value != request.Clan)
                {
                    throw new LogicException(ErrorCode.ClanNotMatched);
                }

                await using var _2 = await LockClans(world, new[] { request.Clan, request.TargetClan });

                var clan = await _dbContext.Clan.Get(world, request.Clan) ??
                    throw new LogicException(ErrorCode.NotFoundClan);

                var requesterMember = await _dbContext.ClanMember.Get(world, clan.Id, requester.Id) ??
                    throw new LogicException(ErrorCode.NotFoundClanMember);

                if (requesterMember.Role != (uint)ClanRole.Master)
                {
                    throw new LogicException(ErrorCode.ClanNoPrivilege);
                }

                var enemy = await _dbContext.ClanEnemy.Get(world, request.Clan, request.TargetClan) ??
                    throw new LogicException(ErrorCode.ClanNotEnemy);

                var reverseEnemy = await _dbContext.ClanEnemy.Get(world, request.TargetClan, request.Clan);

                _dbContext.ClanEnemy.Delete(world, enemy);
                if (reverseEnemy != null)
                {
                    _dbContext.ClanEnemy.Delete(world, reverseEnemy);
                }

                var targetClan = await _dbContext.Clan.Get(world, request.TargetClan);
                if (targetClan != null)
                {
                    Touch(world, clan, targetClan);
                }
                else
                {
                    Touch(world, clan);
                }

                await _dbContext.SaveChangesAsync();

                var response = new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Unenemy,
                    ClanId = clan.Id,
                    ClanName = clan.Name,
                    Revision = clan.Revision,
                    RelatedRevision = targetClan?.Revision ?? 0,
                    Actor = new Protocol.CharacterRef
                    {
                        Uid = requester.Id,
                        Name = requester.Name
                    },
                    RelatedClanId = request.TargetClan,
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.NotifySavedAsync(response, "clan", request.World);
                return response;
            }
            catch (LogicException e)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Unenemy,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedClan
                {
                    Host = request.Host,
                    Action = Protocol.ClanActionType.Unenemy,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}

