using http.Service;
using Microsoft.AspNetCore.Mvc;

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
}
