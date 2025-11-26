using AutoMapper;
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
    [Route("group")]
    public class GroupController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly Fb.Model.Model _model;
        private readonly SessionService _sessionService;
        private readonly RedisService _redisService;
        private readonly RedisDistributedLockService _distributedLock;
        public GroupController(IConfiguration configuration,
            IMapper mapper,
            DbContext dbContext,
            RabbitMqService rabbitMqService,
            Fb.Model.Model model,
            SessionService sessionService,
            RedisService redisService,
            RedisDistributedLockService distributedLock)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _model = model;
            _sessionService = sessionService;
            _redisService = redisService;
            _distributedLock = distributedLock;
        }
        [HttpGet("{id}")]
        public async Task<Response.GetGroup> Get(uint id)
        {
            try
            {
                var redis = _redisService.Redis(id).Connection;

                await using (await _distributedLock.Lock(Group.DistributedLockKey(id)))
                {
                    var group = await _dbContext.Group.Get(id) ??
                        throw new LogicException(ErrorCode.GroupNotFound);

                    var master = await _dbContext.Character.Get(group.Master) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var members = new List<Character>();
                    foreach (var x in group.Members)
                    {
                        var ch = await _dbContext.Character.Get(x) ??
                            throw new LogicException(ErrorCode.NotFoundCharacter);
                        members.Add(ch);
                    }

                    return new Response.GetGroup
                    {
                        Group = new Protocol.Group
                        {
                            Id = group.Master,
                            Master = master.Name,
                            Members = members.ConvertAll(x => x.Name)
                        }
                    };
                }
            }
            catch (LogicException e)
            {
                return new Response.GetGroup
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.GetGroup
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("create")]
        public async Task<Response.EnterGroup> Create(Request.EnterGroup request)
        {
            try
            {
                var actor = await _dbContext.Character.Get(request.Master) ??
                    throw new Exception($"user {request.Master} not found");

                if (actor.Name == request.Member)
                    throw new LogicException(ErrorCode.CannotGroupSelf);

                if (await _sessionService.Get(actor.Name) == null)
                    throw new Exception($"user {request.Master} is offline");

                if (_model.Map.TryGetValue(actor.Map, out var map) == false)
                    throw new Exception("invalid map");

                var targetSession = await _sessionService.Get(request.Member) ??
                    throw new LogicException(ErrorCode.Offline);

                var target = await _dbContext.Character.Get(targetSession.Uid) ??
                    throw new LogicException(ErrorCode.Offline);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(actor.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var actorSync = await _dbContext.CharacterSync.Get(actor.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        await using (await _distributedLock.Lock(Group.DistributedLockKey(actor.Id)))
                        {
                            var isCreated = false;
                            Group group;
                            if (actorSync.Group == null)
                            {
                                // 그룹이 없어서 새로 만드는 경우
                                isCreated = true;
                                group = new Group
                                {
                                    Master = actor.Id,
                                };
                                actorSync.Group = group.Master;
                                _dbContext.CharacterSync.Set(actorSync);
                            }
                            else
                            {
                                // 그룹이 이미 있는 경우
                                if (actorSync.Group.Value != actor.Id)
                                    throw new LogicException(ErrorCode.NotGroupMaster); // 그룹장이 아닌 경우

                                group = await _dbContext.Group.Get(actor.Id);
                            }

                            group.Deleted = false;
                            Protocol.GroupAction action;
                            if (group.Members.Contains(target.Id))
                            {
                                group.Members.Remove(target.Id);
                                targetSync.Group = null;
                                action = Protocol.GroupAction.Kick;
                            }
                            else
                            {
                                var masterSetting = await _dbContext.Option.Get(actor.Id) ??
                                throw new Exception($"user option {request.Master} not found");

                                if (masterSetting.Group == false)
                                    throw new LogicException(ErrorCode.DisabledGroup);

                                var memberSetting = await _dbContext.Option.Get(target.Id) ??
                                    throw new Exception($"user option {request.Member} not found");

                                if (memberSetting.Group == false)
                                    throw new LogicException(ErrorCode.DisabledGroupTarget);

                                if (targetSync.Group != null)
                                    throw new LogicException(ErrorCode.GroupTargetAlreadyJoined);

                                group.Members.Add(target.Id);
                                targetSync.Group = group.Master;
                                action = isCreated ? Protocol.GroupAction.Create : Protocol.GroupAction.Enter;
                            }

                            _dbContext.Group.Set(group);
                            _dbContext.CharacterSync.Set(targetSync);

                            var memberNames = new List<string>();
                            foreach (var uid in group.Members)
                            {
                                var ch = await _dbContext.Character.Get(uid) ??
                                    throw new LogicException(ErrorCode.NotFoundCharacter);

                                memberNames.Add(ch.Name);
                            }

                            var response = new Response.EnterGroup
                            {
                                Group = new Protocol.Group
                                {
                                    Id = group.Master,
                                    Master = actor.Name,
                                    Members = memberNames
                                },
                                Member = request.Member,
                                Host = map.Host,
                                Action = action,
                                Error = 0
                            };

                            await _dbContext.SaveChangesAsync();
                            _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.EnterGroup
                {
                    Member = request.Member,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.EnterGroup
                {
                    Member = request.Member,
                    Error = uint.MaxValue
                };
            }
        }
        [HttpPost("leave")]
        public async Task<Response.LeaveGroup> Leave(Request.LeaveGroup request)
        {
            try
            {
                var session = await _sessionService.Get(request.Member) ??
                    throw new LogicException(ErrorCode.Offline);

                var character = await _dbContext.Character.Get(session.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(character.Id)))
                {
                    var sync = await _dbContext.CharacterSync.Get(character.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    if (_model.Map.TryGetValue(character.Map, out var map) == false)
                        throw new LogicException(ErrorCode.NotFoundMap);

                    var groupId = sync.Group ??
                        throw new LogicException(ErrorCode.GroupNotJoined);

                    await using (await _distributedLock.Lock(Group.DistributedLockKey(groupId)))
                    {
                        var group = await _dbContext.Group.Get(groupId) ??
                        throw new LogicException(ErrorCode.GroupNotFound);

                        if (character.Id == group.Master)
                        {
                            // 그룹장이 길드 해체
                            var memberNames = new List<string>();
                            foreach (var uid in group.Members)
                            {
                                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(uid)))
                                {
                                    var member = await _dbContext.Character.Get(uid) ??
                                        throw new LogicException(ErrorCode.NotFoundCharacter);

                                    var memberSync = await _dbContext.CharacterSync.Get(uid) ??
                                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                                    memberSync.Group = null;
                                    _dbContext.CharacterSync.Set(memberSync);
                                    memberNames.Add(member.Name);
                                }
                            }
                            sync.Group = null;
                            _dbContext.CharacterSync.Set(sync);

                            group.Deleted = true;
                            _dbContext.Group.Set(group);

                            await _dbContext.SaveChangesAsync();

                            var response = new Response.LeaveGroup
                            {
                                Group = new Protocol.Group
                                {
                                    Id = group.Master,
                                    Master = character.Name,
                                    Members = memberNames,
                                },
                                Action = Protocol.GroupAction.BreakUp,
                                Member = request.Member,
                                Host = map.Host
                            };
                            _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                            return response;
                        }
                        else
                        {
                            var master = await _dbContext.Character.Get(group.Master) ??
                                throw new LogicException(ErrorCode.NotFoundCharacter);

                            group.Members.Remove(character.Id);
                            _dbContext.Group.Set(group);

                            sync.Group = null;
                            _dbContext.CharacterSync.Set(sync);

                            var members = new List<Character>();
                            foreach (var uid in group.Members)
                            {
                                members.Add(await _dbContext.Character.Get(uid));
                            }

                            await _dbContext.SaveChangesAsync();

                            var response = new Response.LeaveGroup
                            {
                                Action = Protocol.GroupAction.Leave,
                                Member = request.Member,
                                Group = new Protocol.Group
                                {
                                    Id = group.Master,
                                    Master = master.Name,
                                    Members = members.ConvertAll(x => x.Name)
                                },
                                Host = map.Host
                            };
                            _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.LeaveGroup
                {
                    Member = request.Member,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.LeaveGroup
                {
                    Member = request.Member,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("kick")]
        public async Task<Response.KickGroup> Kick(Request.KickGroup request)
        {
            try
            {
                // Get kicker session and character
                var kickerSession = await _sessionService.Get(request.Kicker) ??
                    throw new LogicException(ErrorCode.Offline);

                var kicker = await _dbContext.Character.Get(kickerSession.Uid) ??
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

                        // Check if kicker is in a group
                        if (kickerSync.Group == null)
                            throw new LogicException(ErrorCode.GroupNotJoined);

                        // Check if target is in the same group
                        if (targetSync.Group != kickerSync.Group)
                            throw new LogicException(ErrorCode.GroupNotJoined);

                        await using (await _distributedLock.Lock(Group.DistributedLockKey(kickerSync.Group.Value)))
                        {
                            var group = await _dbContext.Group.Get(kickerSync.Group.Value) ??
                                throw new LogicException(ErrorCode.GroupNotFound);

                            // Check if kicker is the group master
                            if (group.Master != kicker.Id)
                                throw new LogicException(ErrorCode.NotGroupMaster);

                            // Check if target is a member of the group
                            if (!group.Members.Contains(target.Id))
                                throw new LogicException(ErrorCode.GroupNotJoined);

                            // Remove target from group
                            group.Members.Remove(target.Id);
                            _dbContext.Group.Set(group);

                            targetSync.Group = null;
                            _dbContext.CharacterSync.Set(targetSync);

                            var members = new List<Character>();
                            foreach (var uid in group.Members)
                            {
                                var member = await _dbContext.Character.Get(uid) ??
                                    throw new LogicException(ErrorCode.NotFoundCharacter);
                                members.Add(member);
                            }

                            var memberNames = members.ConvertAll(x => x.Name);

                            var response = new Response.KickGroup
                            {
                                Group = new Protocol.Group
                                {
                                    Id = group.Master,
                                    Master = kicker.Name,
                                    Members = memberNames
                                },
                                Member = target.Name,
                                Action = Protocol.GroupAction.Kick,
                                Host = _model.Map[kicker.Map].Host,
                                Error = 0
                            };

                            await _dbContext.SaveChangesAsync();
                            _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.KickGroup
                {
                    Member = request.Target,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.KickGroup
                {
                    Member = request.Target,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("group")]
        public async Task<Response.BroadcastGroup> Broadcast(Request.BroadcastGroup request)
        {
            try
            {
                await using (await _distributedLock.Lock(Group.DistributedLockKey(request.Group)))
                {
                    var clan = await _dbContext.Group.Get(request.Group) ??
                        throw new LogicException(ErrorCode.GroupNotFound);

                    var response = new Response.BroadcastGroup
                    {
                        Group = request.Group,
                        Message = request.Message,
                        Type = request.Type,
                        Error = (uint)ErrorCode.None
                    };
                    _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                    return response;
                }
            }
            catch (LogicException e)
            {
                return new Response.BroadcastGroup
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.BroadcastGroup
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
            finally
            { }
        }
    }
}