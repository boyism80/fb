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
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly ILogger<BanController> _logger;

        /// <summary>
        /// Initializes a new instance of the <see cref="BanController"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="rabbitMqService">The RabbitMQ service for ban notifications.</param>
        /// <param name="logger">The logger for recording operations and errors.</param>
        public BanController(DbContext dbContext, RabbitMqService rabbitMqService, ILogger<BanController> logger)
        {
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
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
                // Get user ID from name
                var userId = await _dbContext.Character.GetCharacterId(request.Name) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Calculate expire date
                DateTime? expireDate = null;
                if (request.Days.HasValue)
                {
                    expireDate = DateTime.Now.AddDays(request.Days.Value);
                }

                // Create or update ban
                var ban = new Http.Model.Ban
                {
                    User = userId,
                    Reason = request.Reason,
                    ExpireDate = expireDate,
                    CreatedDate = DateTime.Now,
                    UpdatedDate = DateTime.Now
                };

                _dbContext.Ban.Set(ban);
                await _dbContext.SaveChangesAsync();

                // Publish notification via RabbitMQ
                var response = new Response.Ban
                {
                    Name = request.Name,
                    Reason = request.Reason,
                    ExpireDate = expireDate?.ToString("yyyy-MM-dd HH:mm:ss"),
                    Error = (uint)ErrorCode.None
                };
                _rabbitMqService.Publish(response, "amq.direct", "fb.ban");

                _logger.LogInformation("User {Name} (ID: {UserId}) has been banned. Reason: {Reason}, Expire: {ExpireDate}",
                    request.Name, userId, request.Reason, expireDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? "Permanent");

                return response;
            }
            catch (LogicException e)
            {
                return new Response.Ban
                {
                    Name = request.Name,
                    Error = (uint)e.Error
                };
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error banning user {Name}", request.Name);
                return new Response.Ban
                {
                    Name = request.Name,
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
                // Get user ID from name
                var userId = await _dbContext.Character.GetCharacterId(request.Name) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Check if ban exists
                var ban = await _dbContext.Ban.Get(userId);
                if (ban == null)
                    throw new LogicException(ErrorCode.NotFoundBan);

                // Delete ban
                await _dbContext.Ban.Delete(userId);
                await _dbContext.SaveChangesAsync();

                // Publish notification via RabbitMQ
                var response = new Response.Unban
                {
                    Name = request.Name,
                    Error = (uint)ErrorCode.None
                };
                _rabbitMqService.Publish(response, "amq.direct", "fb.ban");

                _logger.LogInformation("User {Name} (ID: {UserId}) has been unbanned.", request.Name, userId);

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

