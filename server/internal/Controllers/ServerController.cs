using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("server")]
    public class ServerController : ControllerBase
    {
        private readonly ILogger<ServerController> _logger;
        private readonly ServerStateService _serverStateService;

        public ServerController(ILogger<ServerController> logger, ServerStateService serverStateService)
        {
            _logger = logger;
            _serverStateService = serverStateService;
        }

        [HttpPost("heartbeat")]
        public async Task<Response.Heartbeat> Heartbeat(Request.Heartbeat request)
        {
            try
            {
                var success = await _serverStateService.UpdateHeartbeat(
                    request.World,
                    request.Service,
                    request.Id,
                    request.Name,
                    request.Ip,
                    request.Port);

                return new Response.Heartbeat
                {
                    Success = success
                };
            }
            catch (Exception e)
            {
                _logger.LogError(e, "heartbeat failed for world {World}, service {Service}, id {Id}", request.World, request.Service, request.Id);
                return new Response.Heartbeat
                {
                    Success = false
                };
            }
        }
    }
}

