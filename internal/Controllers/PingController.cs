using fb.protocol.inter.request;
using fb.protocol.inter.response;
using http.Service;
using Internal.Model.Redis;
using Internal.RedisKey;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;

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
        var config = new HostConfig
        {
            Name = request.Name,
            IP = request.Ip,
            Port = request.Port
        };
        await connection.StringSetAsync(new HeartBeatKey 
        {
            Service = request.Service, 
            Group = request.Group,
        }.Key, JsonConvert.SerializeObject(config), TimeSpan.FromSeconds(5));
        return new Pong
        { };
    }
}
