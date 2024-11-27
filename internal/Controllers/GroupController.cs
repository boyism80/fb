using AutoMapper;
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
        public async Task<Response.CreateGroup> Create(Request.CreateGroup request)
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

                if (member.Group != null)
                    throw new LogicException(ErrorCode.GroupTargetAlreadyJoined);

                var masterSetting = await _dbContext.Option.Get(master.Id) ??
                    throw new Exception($"user option {request.Master} not found");

                if (masterSetting.Group == false)
                    throw new LogicException(ErrorCode.DisabledGroup);

                var memberSetting = await _dbContext.Option.Get(member.Id) ??
                    throw new Exception($"user option {request.Member} not found");

                if (memberSetting.Group == false)
                    throw new LogicException(ErrorCode.DisabledGroupTarget);

                // TODO: 트랜잭션
                var group = master.Group != null ? await _dbContext.Group.Get(master.Group.Value) : null;
                if (group == null)
                {
                    group = new Http.Model.Group
                    {
                        Master = master.Id,
                        Deleted = false
                    };
                    master.Group = group.Master;
                    await _dbContext.Character.Set(master);
                }

                group.Members.Add(member.Id);
                await _dbContext.Group.Set(group);

                member.Group = group.Master;
                await _dbContext.Character.Set(member);

                var memberNames = new List<string>();
                foreach (var uid in group.Members)
                {
                    var ch = await _dbContext.Character.Get(uid) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    memberNames.Add(ch.Name);
                }

                var response = new Response.CreateGroup
                {
                    Group = new Protocol.Group
                    {
                        Id = group.Master,
                        Master = master.Name,
                        Members = memberNames
                    },
                    Member = request.Member,
                    Host = map.Host,
                    Error = 0
                };
                _rabbitMqService.Publish(response, "amq.direct", $"fb.group");

                return response;
            }
            catch (LogicException e)
            {
                return new Response.CreateGroup
                {
                    Member = request.Member,
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
            {
                return new Response.CreateGroup
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

                var member = await _dbContext.Character.Get(session.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (_model.Map.TryGetValue(member.Map, out var map) == false)
                    throw new LogicException(ErrorCode.NotFoundMap);

                var groupId = member.Group ??
                    throw new LogicException(ErrorCode.GroupNotJoined);

                var group = await _dbContext.Group.Get(groupId) ??
                    throw new LogicException(ErrorCode.GroupNotFound);

                if (group.Deleted)
                    throw new LogicException(ErrorCode.GroupNotFound);

                Response.LeaveGroup response;

                // 그룹이 해체되는 경우
                if (group.Members.Count == 0)
                {
                    var master = await _dbContext.Character.Get(group.Master) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var members = new List<Http.Model.Character>();
                    foreach (var uid in group.Members)
                    {
                        var ch = await _dbContext.Character.Get(uid);
                        ch.Group = null;
                        await _dbContext.Character.Set(ch);

                        members.Add(ch);
                    }

                    group.Deleted = true;
                    await _dbContext.Group.Set(group);

                    response = new Response.LeaveGroup
                    {
                        Id = groupId,
                        BreakUp = true,
                        Group = new Protocol.Group
                        {
                            Id = master.Id,
                            Master = master.Name,
                            Members = members.ConvertAll(x => x.Name)
                        },
                        Host = map.Host,
                        Member = request.Member
                    };
                }
                // 그룹장이 탈퇴하면서 새 그룹이 구성되어야하는 경우
                else if (group.Master == member.Id)
                {
                    group.Deleted = true;
                    await _dbContext.Group.Set(group);

                    var replaced = new Http.Model.Group
                    {
                        Master = group.Members.ElementAt(0),
                        Members = group.Members.Skip(1).ToList(),
                        Deleted = false
                    };
                    await _dbContext.Group.Set(replaced);

                    member.Group = null;
                    await _dbContext.Character.Set(member);

                    var master = await _dbContext.Character.Get(group.Master) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var members = new List<Http.Model.Character>();
                    foreach (var x in group.Members)
                    {
                        members.Add(await _dbContext.Character.Get(x) ?? throw new LogicException(ErrorCode.NotFoundCharacter));
                    }

                    foreach (var x in members.Concat([master]))
                    {
                        x.Group = group.Master;
                        await _dbContext.Character.Set(x);
                    }

                    response = new Response.LeaveGroup
                    {
                        Id = groupId,
                        Host = map.Host,
                        Member = request.Member,
                        BreakUp = false,
                        Group = new Protocol.Group
                        {
                            Id = group.Master,
                            Master = master.Name,
                            Members = members.ConvertAll(x => x.Name)
                        }
                    };
                }
                else
                {
                    group.Members.Remove(member.Id);
                    await _dbContext.Group.Set(group);

                    member.Group = null;
                    await _dbContext.Character.Set(member);

                    response = new Response.LeaveGroup
                    {
                        Id = group.Master,
                        Member = request.Member,
                        BreakUp = false,
                        Group = new Protocol.Group
                        {
                            Id = group.Master,
                        },
                        Host = map.Host,
                    };
                }

                _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                return response;
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