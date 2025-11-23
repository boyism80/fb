using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    /// <summary>
    /// Provides ban management operations for the internal API.
    /// Handles user banning and unbanning operations.
    /// </summary>
    [ApiController]
    [Route("ban")]
    public class BanController : ControllerBase
    {
        private readonly BanService _banService;
        private readonly RabbitMqService _rabbitMqService;
        private readonly DbContext _dbContext;
        private readonly ILogger<BanController> _logger;

        /// <summary>
        /// Initializes a new instance of the <see cref="BanController"/> class.
        /// </summary>
        /// <param name="banService">The ban service for ban management operations.</param>
        /// <param name="rabbitMqService">The RabbitMQ service for ban notifications.</param>
        /// <param name="dbContext">The database context for saving changes.</param>
        /// <param name="logger">The logger for recording operations and errors.</param>
        public BanController(BanService banService, RabbitMqService rabbitMqService, DbContext dbContext, ILogger<BanController> logger)
        {
            _banService = banService;
            _rabbitMqService = rabbitMqService;
            _dbContext = dbContext;
            _logger = logger;
        }

        /// <summary>
        /// Bans a user with the specified reason and duration.
        /// If days is null, the ban is permanent.
        /// </summary>
        /// <param name="request">The ban request containing user name, reason, and optional days.</param>
        /// <returns>A response indicating success or error details.</returns>
        [HttpPost("add")]
        public async Task<Response.Ban> Add(Request.Ban request)
        {
            try
            {
                var result = await _banService.BanUser(request.Name, request.Reason, request.Days);
                await _dbContext.SaveChangesAsync();

                var response = new Response.Ban
                {
                    Name = request.Name,
                    Reason = request.Reason,
                    ExpireDate = result.ExpireDate?.ToString("yyyy-MM-dd HH:mm:ss"),
                    Error = (uint)result.Error
                };

                // Publish notification via RabbitMQ
                _rabbitMqService.Publish(response, "amq.direct", "fb.ban");

                return response;
            }
            catch (LogicException e)
            {
                return new Response.Ban
                {
                    Name = request.Name,
                    Reason = request.Reason,
                    Error = (uint)e.Error
                };
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error banning user {Name}", request.Name);
                return new Response.Ban
                {
                    Name = request.Name,
                    Reason = request.Reason,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Removes a ban from a user.
        /// </summary>
        /// <param name="request">The unban request containing user name.</param>
        /// <returns>A response indicating success or error details.</returns>
        [HttpPost("remove")]
        public async Task<Response.Unban> Remove(Request.Unban request)
        {
            try
            {
                var result = await _banService.UnbanUser(request.Name);
                await _dbContext.SaveChangesAsync();

                var response = new Response.Unban
                {
                    Name = request.Name,
                    Error = (uint)result.Error
                };

                // Publish notification via RabbitMQ
                _rabbitMqService.Publish(response, "amq.direct", "fb.unban");

                return response;
            }
            catch (LogicException e)
            {
                return new Response.Unban
                {
                    Name = request.Name,
                    Error = (uint)e.Error
                };
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error unbanning user {Name}", request.Name);
                return new Response.Unban
                {
                    Name = request.Name,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}

