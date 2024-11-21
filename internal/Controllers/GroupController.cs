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
    [Route("group")]
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

        [HttpPost("create")]
        public async Task<Response.CreateGroup> Create(Request.CreateGroup request)
        {
            try
            {
                var master = await _dbContext.Character.Get(request.Master) ??
                    throw new Exception($"user {request.Master} not found");

                if (await _sessionService.Get(master.Name) == null)
                    throw new Exception($"user {request.Master} is offline");

                if (master.Group != null)
                    throw new LogicException(ErrorCode.GroupAlreadyJoined);

                if (_model.Map.TryGetValue(master.Map, out var map) == false)
                    throw new Exception("invalid map");

                var member = await _dbContext.Character.Get(request.Member) ??
                    throw new Exception($"user {request.Member} not found");

                if (await _sessionService.Get(member.Name) == null)
                    throw new LogicException(ErrorCode.Offline);

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

                // TODO: ∆Æ∑£¿Ëº«
                var group = new Http.Model.Group
                {
                    Master = master.Id,
                    Members = [member.Id]
                };
                await _dbContext.Group.Set(group);

                master.Group = group.Master;
                await _dbContext.Character.Set(master);

                member.Group = group.Master;
                await _dbContext.Character.Set(member);

                var response = new Response.CreateGroup
                {
                    Group = new Protocol.Group
                    {
                        Master = master.Id,
                        Members = [new Protocol.GroupMember
                        {
                            Uid = member.Id,
                            Name = member.Name
                        }]
                    },
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
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
            {
                return new Response.CreateGroup
                {
                    Error = uint.MaxValue
                };
            }
        }

        [HttpPost("leave")]
        public async Task<Response.LeaveGroup> Leave(Request.LeaveGroup request)
        {
            throw new NotImplementedException();
        }

        [HttpPost("enter")]
        public async Task<Response.EnterGroup> Enter(Request.EnterGroup request)
        {
            throw new NotImplementedException();
        }
    }
}