using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("mail/system")]
    public class SystemMailController : ControllerBase
    {
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;

        public SystemMailController(DbContext dbContext, RabbitMqService rabbitMqService)
        {
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
        }

        [HttpGet]
        public async Task<Response.GetSystemMails> GetSystemMails()
        {
            try
            {
                var systemMails = await _dbContext.SystemMail.GetAll();
                var protocolMails = systemMails.Select(m => new Protocol.SystemMail
                {
                    Id = m.Id,
                    Sender = m.Sender,
                    Title = m.Title,
                    Contents = m.Contents,
                    ExpireDate = m.ExpireDate?.ToString("yyyy-MM-dd HH:mm:ss"),
                    CreatedDate = m.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")
                }).ToList();

                return new Response.GetSystemMails
                {
                    Mails = protocolMails,
                    Error = (uint)ErrorCode.None
                };
            }
            catch (Exception)
            {
                return new Response.GetSystemMails
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost]
        public async Task<Response.WriteSystemMail> WriteSystemMail(Request.WriteSystemMail request)
        {
            try
            {
                DateTime? expireDate = null;
                if (!string.IsNullOrEmpty(request.ExpireDate) && DateTime.TryParse(request.ExpireDate, out var parsedDate))
                    expireDate = parsedDate;

                var systemMail = await _dbContext.SystemMail.Write(request.Sender, request.Title, request.Contents, expireDate);
                var protocolMail = new Protocol.SystemMail
                {
                    Id = systemMail.Id,
                    Sender = systemMail.Sender,
                    Title = systemMail.Title,
                    Contents = systemMail.Contents,
                    ExpireDate = systemMail.ExpireDate?.ToString("yyyy-MM-dd HH:mm:ss"),
                    CreatedDate = systemMail.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")
                };

                var response = new Response.WriteSystemMail
                {
                    Mail = protocolMail,
                    Error = (uint)ErrorCode.None
                };

                _rabbitMqService.Publish(response, "amq.direct", "fb.system");
                return response;
            }
            catch (Exception)
            {
                return new Response.WriteSystemMail
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}

