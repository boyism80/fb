using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("system")]
    public class SystemController : ControllerBase
    {
        private readonly RedisService _redisService;
        private readonly ILogger _logger;
        private readonly RabbitMqService _rabbitMqService;

        public SystemController(RedisService redisService,
            ILogger<SystemController> logger,
            RabbitMqService rabbitMqService)
        {
            _redisService = redisService;
            _logger = logger;
            _rabbitMqService = rabbitMqService;
        }

        [HttpPost("shutdown")]
        public async Task Shutdown()
        {
            _rabbitMqService.Publish(new Response.Shutdown
            {

            }, "amq.direct", $"fb.system");
        }
    }
}