using AutoMapper;
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
    [Route("mail")]
    public class MailController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        public MailController(IConfiguration configuration, IMapper mapper, DbContext dbContext, RabbitMqService rabbitMqService)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
        }
        [HttpGet("{user}")]
        public async Task<Response.GetMailList> GetMailList(uint user, [FromQuery(Name = "offset")] ushort offset, [FromQuery(Name = "count")] ushort count)
        {
            try
            {
                var mails = await _dbContext.Mail.GetList(user, offset, count);
                var summaryList = _mapper.Map<List<Http.Model.Mail>, List<Protocol.MailSummary>>(mails.ToList());

                // Resolve sender names from UIDs (name table is in global DB, cannot JOIN)
                var senderIds = mails.Select(m => m.Sender).Distinct().ToList();
                var senderNames = await _dbContext.Character.GetName(senderIds);

                // Fill sender names into protocol objects
                for (int i = 0; i < summaryList.Count; i++)
                {
                    summaryList[i].Sender = senderNames.GetValueOrDefault(mails[i].Sender) ?? string.Empty;
                }

                return new Response.GetMailList
                {
                    SummaryList = summaryList
                };
            }
            catch (LogicException e)
            {
                return new Response.GetMailList
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.GetMailList
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpGet("{user}/{id}")]
        public async Task<Response.GetMail> GetMail(uint user, ushort id)
        {
            try
            {
                var mail = await _dbContext.Mail.Get(user, id) ??
                    throw new LogicException(ErrorCode.NotFoundMail);

                var protocolMail = _mapper.Map<Protocol.Mail>(mail);

                // Resolve sender name from UID (name table is in global DB, cannot JOIN)
                var senderName = await _dbContext.Character.GetName(mail.Sender) ?? string.Empty;
                protocolMail.Sender = senderName;

                return new Response.GetMail
                {
                    Mail = protocolMail,
                    Unread = await _dbContext.Mail.Unread(user),
                    Error = (uint)ErrorCode.None,
                };
            }
            catch (LogicException e)
            {
                return new Response.GetMail
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.GetMail
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("write")]
        public async Task<Response.WriteMail> Write(Request.WriteMail request)
        {
            try
            {
                var mail = await _dbContext.Mail.Write(request.User, request.Sender, request.Title, request.Contents);
                var protocolMail = _mapper.Map<Protocol.Mail>(mail);

                // Resolve sender name from UID (name table is in global DB, cannot JOIN)
                var senderName = await _dbContext.Character.GetName(mail.Sender) ?? string.Empty;
                protocolMail.Sender = senderName;

                var response = new Response.WriteMail
                {
                    Mail = protocolMail,
                    Host = request.Host,
                    Unread = await _dbContext.Mail.Unread(mail.User),
                    Error = (uint)ErrorCode.None
                };

                _rabbitMqService.Publish(response, "amq.direct", $"fb.mail");
                return response;
            }
            catch (LogicException e)
            {
                return new Response.WriteMail
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.WriteMail
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("delete")]
        public async Task<Response.DeleteMail> Delete(Request.DeleteMail request)
        {
            try
            {
                var success = await _dbContext.Mail.Delete(request.User, request.Id);
                if (!success)
                    throw new LogicException(ErrorCode.MailNotExists);

                return new Response.DeleteMail
                {
                    Unread = await _dbContext.Mail.Unread(request.User),
                    Error = (uint)ErrorCode.None
                };
            }
            catch (LogicException e)
            {
                return new Response.DeleteMail
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.DeleteMail
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
        [HttpPost("system/get")]
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
        [HttpPost("system/write")]
        public async Task<Response.WriteSystemMail> WriteSystemMail(Request.WriteSystemMail request)
        {
            try
            {
                DateTime? expireDate = null;
                if (!string.IsNullOrEmpty(request.ExpireDate))
                {
                    if (DateTime.TryParse(request.ExpireDate, out var parsedDate))
                        expireDate = parsedDate;
                }

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

                // Publish notification to all game servers via RabbitMQ
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