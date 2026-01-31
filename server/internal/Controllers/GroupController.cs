using Internal.Services;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("group")]
    public class GroupController : ControllerBase
    {
        private readonly GroupService _groupService;

        public GroupController(GroupService groupService)
        {
            _groupService = groupService;
        }

        [HttpGet("{world}/{id}")]
        public async Task<Response.GroupDetails> Get(uint world, uint id)
        {
            return await _groupService.Get(world, id);
        }

        [HttpPost("create")]
        public async Task<Response.GroupDetails> Create(Request.CreateGroup request)
        {
            return await _groupService.Create(request);
        }

        [HttpPost("enter")]
        public async Task<Response.UpdatedGroup> Enter(Request.EnterGroup request)
        {
            return await _groupService.Enter(request);
        }

        [HttpPost("leave")]
        public async Task<Response.UpdatedGroup> Leave(Request.LeaveGroup request)
        {
            return await _groupService.Leave(request);
        }

        [HttpPost("kick")]
        public async Task<Response.UpdatedGroup> Kick(Request.KickGroup request)
        {
            return await _groupService.Kick(request);
        }

        [HttpPost("destroy")]
        public async Task<Response.DestroyGroup> Destroy(Request.DestroyGroup request)
        {
            return await _groupService.Destroy(request);
        }

        [HttpPost("broadcast")]
        public async Task<Response.BroadcastGroup> Broadcast(Request.BroadcastGroup request)
        {
            return await _groupService.Broadcast(request);
        }

        [HttpPost("toggle")]
        public async Task<Response.UpdatedGroup> Toggle(Request.EnterGroup request)
        {
            return await _groupService.Toggle(request);
        }
    }
}
