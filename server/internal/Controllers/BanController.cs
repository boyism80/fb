using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("ban")]
    public class BanController : ControllerBase
    {
        private readonly BanService _banService;
        private readonly RabbitMqService _rabbitMqService;
        private readonly DbContext _dbContext;
        private readonly ILogger<BanController> _logger;
        public BanController(BanService banService, RabbitMqService rabbitMqService, DbContext dbContext, ILogger<BanController> logger)
        {
            _banService = banService;
            _rabbitMqService = rabbitMqService;
            _dbContext = dbContext;
            _logger = logger;
        }
        [HttpPost("add")]
        public async Task<Response.Ban> Add(Request.Ban request)
        {
            try
            {
                var result = await _banService.Ban(request.Name, request.Reason, request.Days);
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
        [HttpPost("remove")]
        public async Task<Response.Unban> Remove(Request.Unban request)
        {
            try
            {
                var result = await _banService.Unban(request.Name);
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

