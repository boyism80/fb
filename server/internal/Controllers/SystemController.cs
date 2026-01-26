using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
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
        private readonly IConfiguration _configuration;
        
        public SystemController(RedisService redisService,
            ILogger<SystemController> logger,
            RabbitMqService rabbitMqService,
            IConfiguration configuration)
        {
            _redisService = redisService;
            _logger = logger;
            _rabbitMqService = rabbitMqService;
            _configuration = configuration;
        }
        
        [HttpPost("shutdown")]
        public Task Shutdown()
        {
            // Get first section for global shutdown message
            var rabbitMqSection = _configuration.GetSection("RabbitMQ");
            var firstSection = rabbitMqSection.GetChildren().FirstOrDefault()?.Key ?? "section-1";
            
            _rabbitMqService.Publish(firstSection, new Response.Shutdown
            {

            }, "amq.direct", $"fb.system");
            return Task.CompletedTask;
        }
    }
}