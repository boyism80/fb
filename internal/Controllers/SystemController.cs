using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    /// <summary>
    /// Provides system-level operations and administrative functions for the internal API.
    /// Handles system shutdown and other administrative tasks.
    /// </summary>
    [ApiController]
    [Route("system")]
    public class SystemController : ControllerBase
    {
        private readonly RedisService _redisService;
        private readonly ILogger _logger;
        private readonly RabbitMqService _rabbitMqService;

        /// <summary>
        /// Initializes a new instance of the <see cref="SystemController"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="logger">The logger for recording system operations.</param>
        /// <param name="rabbitMqService">The RabbitMQ service for message publishing.</param>
        public SystemController(RedisService redisService,
            ILogger<SystemController> logger,
            RabbitMqService rabbitMqService)
        {
            _redisService = redisService;
            _logger = logger;
            _rabbitMqService = rabbitMqService;
        }

        /// <summary>
        /// Initiates a graceful shutdown of the system by publishing a shutdown message.
        /// Sends a shutdown signal via RabbitMQ to all connected services.
        /// </summary>
        /// <returns>A task representing the asynchronous shutdown operation.</returns>
        [HttpPost("shutdown")]
        public async Task Shutdown()
        {
            _rabbitMqService.Publish(new Response.Shutdown
            {

            }, "amq.direct", $"fb.system");
        }
    }
}