using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("system")]
    public class SystemController : ControllerBase
    {
        private readonly ShutdownService _shutdown;

        public SystemController(ShutdownService shutdown)
        {
            _shutdown = shutdown;
        }

        [HttpPost("shutdown")]
        public async Task<Response.Shutdown> Shutdown(Request.Shutdown request)
        {
            await _shutdown.Shutdown(request?.Actor ?? string.Empty, "game");
            return new Response.Shutdown();
        }
    }
}
