﻿using Microsoft.AspNetCore.Mvc;
using RabbitMQ.Client;
using System.Text;

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
        public RabbitMqController()
        {
            
        }

        [HttpPost]
        public async Task Post(Publish request)
        {
            var factory = new ConnectionFactory() { HostName = "192.168.0.181", Port = 5672, UserName = "fb", Password = "admin" };
            using var connection = factory.CreateConnection();
            using var channel = connection.CreateModel();

            var body = Encoding.UTF8.GetBytes(request.Message);
            channel.BasicPublish(exchange: request.ExchangeName, routingKey: request.RouteKey, basicProperties: null, body: body);
        }
    }
}
