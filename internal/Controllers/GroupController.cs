using AutoMapper;
using fb.protocol._internal;
using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("in-game/group")]
    public class GroupController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly Fb.Model.Model _model;
        private readonly SessionService _sessionService;

        public GroupController(IConfiguration configuration,
            IMapper mapper,
            DbContext dbContext,
            RabbitMqService rabbitMqService,
            Fb.Model.Model model,
            SessionService sessionService)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _model = model;
            _sessionService = sessionService;
        }

        [HttpGet("{id}")]
        public async Task<Response.GetGroup> Get(uint id)
        {
            try
            {
                var group = await _dbContext.Group.Get(id) ??
                    throw new LogicException(ErrorCode.GroupNotFound);

                var master = await _dbContext.Character.Get(group.Master) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var members = new List<Http.Model.Character>();
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
            catch (LogicException e)
            {
                return new Response.GetGroup
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
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
                    throw new Exception($"user {request.Member} not found");

                var group = master.Group != null ? await _dbContext.Group.Get(master.Group.Value) : null;
                var isCreated = false;
                if (group == null)
                {
                    group = new Http.Model.Group
                    {
                        Master = master.Id,
                    };
                    master.Group = group.Master;
                    _dbContext.Character.Set(master);
                    isCreated = true;
                }
                group.Deleted = false;

                if (group.Master != master.Id)
                    throw new LogicException(ErrorCode.NotGroupMaster);

                GroupAction action;
                if (group.Members.Contains(member.Id))
                {
                    group.Members.Remove(member.Id);
                    member.Group = null;
                    action = GroupAction.Kick;
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

                    if (member.Group != null)
                        throw new LogicException(ErrorCode.GroupTargetAlreadyJoined);

                    group.Members.Add(member.Id);
                    member.Group = group.Master;
                    action = isCreated ? GroupAction.Create : GroupAction.Enter;
                }

                _dbContext.Group.Set(group);
                _dbContext.Character.Set(member);

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

                if (_model.Map.TryGetValue(character.Map, out var map) == false)
                    throw new LogicException(ErrorCode.NotFoundMap);

                var groupId = character.Group ??
                    throw new LogicException(ErrorCode.GroupNotJoined);

                var group = await _dbContext.Group.Get(groupId) ??
                    throw new LogicException(ErrorCode.GroupNotFound);

                if (group.Deleted)
                    throw new LogicException(ErrorCode.GroupNotFound);

                if (character.Id == group.Master)
                {
                    // 그룹장이 길드 해체
                    var meemberNames = new List<string>();
                    foreach (var uid in group.Members)
                    {
                        var member = await _dbContext.Character.Get(uid) ??
                            throw new LogicException(ErrorCode.NotFoundCharacter);

                        member.Group = null;
                        _dbContext.Character.Set(member);
                        meemberNames.Add(member.Name);
                    }
                    character.Group = null;
                    _dbContext.Character.Set(character);

                    group.Deleted = true;
                    _dbContext.Group.Set(group);

                    await _dbContext.SaveChangesAsync();

                    var response = new Response.LeaveGroup
                    {
                        Group = new Group
                        {
                            Id = group.Master,
                            Master = character.Name,
                            Members = meemberNames,
                        },
                        Action = GroupAction.BreakUp,
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

                    character.Group = null;
                    _dbContext.Character.Set(character);

                    var members = new List<Http.Model.Character>();
                    foreach (var uid in group.Members)
                    {
                        members.Add(await _dbContext.Character.Get(uid));
                    }

                    await _dbContext.SaveChangesAsync();

                    var response = new Response.LeaveGroup
                    {
                        Action = GroupAction.Leave,
                        Member = request.Member,
                        Group = new Group
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
    }
}