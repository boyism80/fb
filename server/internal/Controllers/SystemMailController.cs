using Fb.Model.EnumValue;
using Http.Filters;
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
        private readonly LogService _logService;

        public SystemMailController(DbContext dbContext, LogService logService)
        {
            _dbContext = dbContext;
            _logService = logService;
        }

        [HttpGet("{world}")]
        [SuppressRequestLog]
        public async Task<Response.GetSystemMails> GetSystemMails(uint world, [FromQuery(Name = "offset")] uint offset = 0)
        {
            try
            {
                var systemMails = await _dbContext.SystemMail.GetAll(world, offset);
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

                var systemMail = await _dbContext.SystemMail.Write(request.World, request.Sender, request.Title, request.Contents, expireDate);
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

                // Log system mail creation event
                await _logService.WriteAsync("system_mail_create", new
                {
                    mail_id = systemMail.Id,
                    sender_id = systemMail.Sender,
                    expire_date = systemMail.ExpireDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? null
                });

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

