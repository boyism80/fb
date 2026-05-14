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
        [HttpGet("{world}/{user}")]
        public async Task<Response.GetMailList> GetMailList(uint world, uint user, [FromQuery(Name = "offset")] ushort offset, [FromQuery(Name = "count")] ushort count)
        {
            try
            {
                var mails = await _dbContext.Mail.GetList(world, user, offset, count);
                var summaryList = _mapper.Map<List<Http.Model.Mail>, List<Protocol.MailSummary>>(mails.ToList());
                if (summaryList.Count > 0)
                {
                    // Resolve sender names from UIDs (name table is in global DB, cannot JOIN)
                    var senderIds = mails.Select(m => m.Sender).Distinct().ToList();
                    var senderNames = await _dbContext.Character.GetName(world, senderIds);

                    // Fill sender names into protocol objects
                    for (int i = 0; i < summaryList.Count; i++)
                    {
                        summaryList[i].Sender = senderNames.GetValueOrDefault(mails[i].Sender) ?? string.Empty;
                    }
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
        [HttpGet("{world}/{user}/{id}")]
        public async Task<Response.GetMail> GetMail(uint world, uint user, ushort id)
        {
            try
            {
                var mail = await _dbContext.Mail.Get(world, user, id) ??
                    throw new LogicException(ErrorCode.NotFoundMail);

                var protocolMail = _mapper.Map<Protocol.Mail>(mail);

                // Resolve sender name from UID (name table is in global DB, cannot JOIN)
                var senderName = await _dbContext.Character.GetName(world, mail.Sender) ?? string.Empty;
                protocolMail.Sender = senderName;

                return new Response.GetMail
                {
                    Mail = protocolMail,
                    Unread = await _dbContext.Mail.Unread(world, user),
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
                var mail = await _dbContext.Mail.Write(request.World, request.User, request.Sender, request.Title, request.Contents);
                var protocolMail = _mapper.Map<Protocol.Mail>(mail);

                // Resolve sender name from UID (name table is in global DB, cannot JOIN)
                var senderName = await _dbContext.Character.GetName(request.World, mail.Sender) ?? string.Empty;
                protocolMail.Sender = senderName;

                var response = new Response.WriteMail
                {
                    Mail = protocolMail,
                    Host = request.Host,
                    Unread = await _dbContext.Mail.Unread(request.World, mail.User),
                    Error = (uint)ErrorCode.None
                };

                await _rabbitMqService.PublishAsync(response, "amq.direct", $"fb.{request.World}.mail");
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
                var success = await _dbContext.Mail.Delete(request.World, request.User, request.Id);
                if (!success)
                    throw new LogicException(ErrorCode.MailNotExists);

                return new Response.DeleteMail
                {
                    Unread = await _dbContext.Mail.Unread(request.World, request.User),
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
    }
}