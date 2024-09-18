using fb.protocol.inter.request;
using fb.protocol.inter.response;
using http.Service;
using Microsoft.AspNetCore.Mvc;

namespace Internal.Controllers;
[ApiController]
[Route("[controller]")]
public class PingController : ControllerBase
{
    private readonly ILogger<PingController> _logger;
    private readonly RedisService _redisService;

    public PingController(ILogger<PingController> logger, RedisService redisService)
    {
        _logger = logger;
        _redisService = redisService;
    }

    [HttpPost]
    public async Task<Pong> Ping(Ping request)
    {
        var connection = _redisService.Connection;
        await connection.StringSetAsync($"{request.Service}:{request.Group}", request.Name, TimeSpan.FromSeconds(5));
        return new Pong
        { };
    }
}
