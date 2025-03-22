using AutoMapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Medallion.Threading.Redis;
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
                var master = await _dbContext.Character.Get(request.Master) ??
                    throw new Exception($"user {request.Master} not found");

                if (master.Name == request.Member)
                    throw new LogicException(ErrorCode.CannotGroupSelf);

                if (await _sessionService.Get(master.Name) == null)
                    throw new Exception($"user {request.Master} is offline");

                if (_model.Map.TryGetValue(master.Map, out var map) == false)
                    throw new Exception("invalid map");

                var memberSession = await _sessionService.Get(request.Member) ??
                    throw new LogicException(ErrorCode.Offline);

                var member = await _dbContext.Character.Get(memberSession.Uid) ??
                    throw new LogicException(ErrorCode.Offline);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(master.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(member.Id)))
                    {
                        var masterSync = await _dbContext.CharacterSync.Get(master.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var memberSync = await _dbContext.CharacterSync.Get(member.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        await using (await _distributedLock.Lock(Group.DistributedLockKey(master.Id)))
                        {
                            var group = masterSync.Group != null ? await _dbContext.Group.Get(master.Id) : null;
                            var isCreated = false;
                            if (group == null)
                            {
                                group = new Group
                                {
                                    Master = master.Id,
                                };
                                masterSync.Group = group.Master;
                                _dbContext.CharacterSync.Set(masterSync);
                                isCreated = true;
                            }
                            group.Deleted = false;

                            if (group.Master != master.Id)
                                throw new LogicException(ErrorCode.NotGroupMaster);

                            Protocol.GroupAction action;
                            if (group.Members.Contains(member.Id))
                            {
                                group.Members.Remove(member.Id);
                                memberSync.Group = null;
                                action = Protocol.GroupAction.Kick;
                            }
                            else
                            {
                                var masterSetting = await _dbContext.Option.Get(master.Id) ??
                                throw new Exception($"user option {request.Master} not found");

                                if (masterSetting.Group == false)
                                    throw new LogicException(ErrorCode.DisabledGroup);

                                var memberSetting = await _dbContext.Option.Get(member.Id) ??
                                    throw new Exception($"user option {request.Member} not found");

                                if (memberSetting.Group == false)
                                    throw new LogicException(ErrorCode.DisabledGroupTarget);

                                if (memberSync.Group != null)
                                    throw new LogicException(ErrorCode.GroupTargetAlreadyJoined);

                                group.Members.Add(member.Id);
                                memberSync.Group = group.Master;
                                action = isCreated ? Protocol.GroupAction.Create : Protocol.GroupAction.Enter;
                            }

                            _dbContext.Group.Set(group);
                            _dbContext.CharacterSync.Set(memberSync);

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
                                    Master = master.Name,
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
            catch (Exception e)
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
            catch (Exception e)
            {
                return new Response.LeaveGroup
                {
                    Member = request.Member,
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