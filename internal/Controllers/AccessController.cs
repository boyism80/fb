using fb.protocol.inter;
using fb.protocol.inter.response;
using http.Service;
using Internal.Model.Redis;
using Internal.Redis;
using Internal.Redis.Key;
using Internal.Service;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using StackExchange.Redis;
using request = fb.protocol.inter.request;
using response = fb.protocol.inter.response;

namespace Internal.Controllers;
[ApiController]
[Route("[controller]")]
public class AccessController : ControllerBase
{
    private readonly ILogger<TransferController> _logger;
    private readonly RedisService _redisService;
    private readonly Fb.Model.Model _dataSet;
    private readonly RabbitMqService _rabbitMqService;

    public AccessController(ILogger<TransferController> logger,
        RedisService redisService,
        Fb.Model.Model dataSet,
        RabbitMqService rabbitMqService)
    {
        _logger = logger;
        _redisService = redisService;
        _dataSet = dataSet;
        _rabbitMqService = rabbitMqService;
    }

    [HttpPost("login")]
    public async Task<response.Login> Login(request.Login request)
    {
        var map = _dataSet.Map[request.Map] ?? throw new KeyNotFoundException($"{request.Map} not found in map data");
        var connection = _redisService.Connection;
        var config = await connection.JsonGetAsync<HostConfig>(new HeartBeatKey { Service = fb.protocol.inter.Service.Game, Id = map.Host }.Key);
        if (config == null)
            return new response.Login { Success = false, Logon = false };

        var redisResult = await connection.ScriptEvaluateAsync("login.lua", new 
        {
            key = new RedisKey(new LoginKey { }.Key),
            name = request.Name,
            session = JsonConvert.SerializeObject(new Session 
            {
                Uid = request.Uid,
                Host = map.Host
            }),
        });

        var success = (bool)redisResult[0];
        if (success == false)
        {
            var session = JsonConvert.DeserializeObject<Session>(redisResult[1].ToString());
            _rabbitMqService.Publish(new KickOut 
            {
                Uid = session.Uid
            }, "amq.direct", $"fb.game.{session.Host}");
            return new response.Login { Success = false, Logon = true };
        }

        return new response.Login
        {
            Success = success,
            Logon = false,
            Ip = config.IP,
            Port = config.Port
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
