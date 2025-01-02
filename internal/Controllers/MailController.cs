using AutoMapper;
using Dapper;
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
        public async Task<Response.GetMailList> GetMailList(uint user, [FromQuery(Name = "offset")] ushort offset)
        {
            try
            {
                var mails = await _dbContext.Mail.GetList(user, offset);
                var summaryList = _mapper.Map<List<Http.Model.Mail>, List<Protocol.MailSummary>>(mails.ToList());
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

                return new Response.GetMail
                {
                    Mail = _mapper.Map<Protocol.Mail>(mail),
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
            catch (Exception e)
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
                var response = new Response.WriteMail
                {
                    Mail = _mapper.Map<Protocol.Mail>(mail),
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
                await _dbContext.Mail.Delete(request.User, request.Id);
                return new Response.DeleteMail
                {
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