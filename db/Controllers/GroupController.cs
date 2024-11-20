using AutoMapper;
using Db;
using Db.Model;
using Db.Service;
using http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol.db.request;
using Response = fb.protocol.db.response;

namespace db.Controllers
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

        public GroupController(IConfiguration configuration,
            IMapper mapper,
            DbContext dbContext,
            RabbitMqService rabbitMqService,
            Fb.Model.Model model)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _model = model;
        }

        [HttpPost("create")]
        public async Task<Response.CreateGroup> Create(Request.CreateGroup request)
        {
            try
            {
                // TODO: 접속중인거 확인

                var master = await _dbContext.Character.Get(request.Master) ??
                    throw new Exception($"user {request.Master} not found");

                if (master.Group != null)
                    throw new GroupException(1);

                if (_model.Map.TryGetValue(master.Map, out var map) == false)
                    throw new Exception("invalid map");

                var member = await _dbContext.Character.Get(request.Member) ??
                    throw new Exception($"user {request.Member} not found");

                if (member.Group != null)
                    throw new GroupException(2);

                var masterSetting = await _dbContext.Option.Get(master.Id) ??
                    throw new Exception($"user option {request.Master} not found");

                if (masterSetting.Group == false)
                    throw new GroupException(3);

                var memberSetting = await _dbContext.Option.Get(member.Id) ??
                    throw new Exception($"user option {request.Member} not found");

                if (memberSetting.Group == false)
                    throw new GroupException(4);

                // TODO: 트랜잭션
                var group = new Group
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
                    Group = new fb.protocol.db.Group
                    {
                        Master = master.Id,
                        Members = [new fb.protocol.db.GroupMember
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
            catch (GroupException e)
            {
                return new Response.CreateGroup
                {
                    Error = e.Error
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