using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("ops")]
    public class OpsController : ControllerBase
    {
        private readonly OpsNotifyService _ops;

        public OpsController(OpsNotifyService ops)
        {
            _ops = ops;
        }

        [HttpPost("notify")]
        public async Task<Response.OpsNotify> Notify(Request.OpsNotify request)
        {
            await _ops.Notify(
                request.Action,
                request.Source,
                request.Actor,
                request.Target,
                request.World,
                request.Detail);
            return new Response.OpsNotify();
        }
    }
}
