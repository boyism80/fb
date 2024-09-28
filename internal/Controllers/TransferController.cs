using fb.protocol.inter;
using http.Service;
using Internal.Model.Redis;
using Internal.Redis.Key;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using request = fb.protocol.inter.request;
using response = fb.protocol.inter.response;

namespace Internal.Controllers;
[ApiController]
[Route("[controller]")]
public class TransferController : ControllerBase
{
    private readonly ILogger<TransferController> _logger;
    private readonly RedisService _redisService;

    public TransferController(ILogger<TransferController> logger,
        RedisService redisService)
    {
        _logger = logger;
        _redisService = redisService;
    }

    [HttpPost]
    public async Task<response.Transfer> Transfer(request.Transfer request)
    {
        var connection = _redisService.Connection;
        var exists = await connection.StringGetAsync(new HeartBeatKey { Service = request.Service, Id = request.Id }.Key);
        if (exists.IsNull)
        {
            return new response.Transfer
            {
                Code = TransferResult.Failed,
                Ip = "0.0.0.0",
            };
        }

        var config = JsonConvert.DeserializeObject<HostConfig>(exists.ToString());
        return new response.Transfer
        {
            Code = TransferResult.Success,
            Ip = config.IP,
            Port = config.Port
        };
    }
}
