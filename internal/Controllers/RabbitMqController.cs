using fb.protocol.inter.response;
using Internal.Service;
using Microsoft.AspNetCore.Mvc;
using RabbitMQ.Client;

namespace Internal.Controllers
{
    public class Publish
    {
        public string ExchangeName { get; set; }
        public string Type { get; set; } = ExchangeType.Direct;
        public string RouteKey { get; set; } = string.Empty;
        public string Message { get; set; }
    }

    [ApiController]
    [Route("[controller]")]
    public class RabbitMqController : ControllerBase
    {
        private readonly RabbitMqService _rabbitMqService;

        public RabbitMqController(RabbitMqService rabbitMqService)
        {
            _rabbitMqService = rabbitMqService;
        }

        [HttpPost]
        public async Task Post(Publish request)
        {
            _rabbitMqService.Publish(new Pong(), request.ExchangeName, request.RouteKey);
        }
    }
}
