using fb.protocol._internal.response;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using RabbitMQ.Client;

namespace Internal.Controllers
{
    /// <summary>
    /// Represents a message publishing request model for RabbitMQ operations.
    /// Contains all necessary information to publish a message to a RabbitMQ exchange.
    /// </summary>
    public class Publish
    {
        /// <summary>
        /// Gets or sets the name of the RabbitMQ exchange to publish to.
        /// </summary>
        /// <value>The exchange name where the message will be published.</value>
        public string ExchangeName { get; set; }

        /// <summary>
        /// Gets or sets the type of the RabbitMQ exchange.
        /// </summary>
        /// <value>The exchange type (e.g., Direct, Topic, Fanout). Default is Direct.</value>
        public string Type { get; set; } = ExchangeType.Direct;

        /// <summary>
        /// Gets or sets the routing key for message delivery.
        /// </summary>
        /// <value>The routing key used to determine message routing. Default is empty string.</value>
        public string RouteKey { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the message content to be published.
        /// </summary>
        /// <value>The message payload as a string.</value>
        public string Message { get; set; }
    }

    /// <summary>
    /// Provides RabbitMQ message publishing operations for the internal API.
    /// Handles message queue operations and communication with external services.
    /// </summary>
    [ApiController]
    [Route("[controller]")]
    public class RabbitMqController : ControllerBase
    {
        private readonly RabbitMqService _rabbitMqService;

        /// <summary>
        /// Initializes a new instance of the <see cref="RabbitMqController"/> class.
        /// </summary>
        /// <param name="rabbitMqService">The RabbitMQ service for message publishing operations.</param>
        public RabbitMqController(RabbitMqService rabbitMqService)
        {
            _rabbitMqService = rabbitMqService;
        }
    }
}
