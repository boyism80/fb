using http.Service;
using Internal.RedisKey;
using Microsoft.AspNetCore.Mvc;
using request = fb.protocol.inter.request;
using response = fb.protocol.inter.response;

namespace Internal.Controllers;
[ApiController]
[Route("[controller]")]
public class AccessController : ControllerBase
{
    private readonly ILogger<TransferController> _logger;
    private readonly RedisService _redisService;

    public AccessController(ILogger<TransferController> logger,
        RedisService redisService)
    {
        _logger = logger;
        _redisService = redisService;
    }

    [HttpPost("login")]
    public async Task<response.Login> Login(request.Login request)
    {
        var connection = _redisService.Connection;
        await connection.HashSetAsync(new LoginKey { }.Key, $"{request.Uid}", true);
        return new response.Login
        {
            Uid = request.Uid
        };
    }

    [HttpPost("logout")]
    public async Task<response.Logout> Logout(request.Logout request)
    {
        var connection = _redisService.Connection;
        await connection.HashDeleteAsync(new LoginKey { }.Key, $"{request.Uid}");

        return new response.Logout
        { 
            Uid = request.Uid
        };
    }
}
