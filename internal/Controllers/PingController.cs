using fb.protocol.flatbuffer.inter.request.model;
using fb.protocol.flatbuffer.inter.response.model;
using Microsoft.AspNetCore.Mvc;

namespace fb.protocol.flatbuffer.inter;
[ApiController]
[Route("[controller]")]
public class PingController : ControllerBase
{
    private readonly ILogger<PingController> _logger;

    public PingController(ILogger<PingController> logger)
    {
        _logger = logger;
    }

    [HttpPost]
    public Pong Get(Ping request)
    {
        _logger.LogInformation("ping");
        return new Pong
        { };
    }
}
