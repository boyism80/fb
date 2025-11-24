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
    /// <summary>
    /// Provides mail system operations for the internal API.
    /// Handles mail retrieval, writing, and deletion for the in-game mail system.
    /// </summary>
    [ApiController]
    [Route("mail")]
    public class MailController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;

        /// <summary>
        /// Initializes a new instance of the <see cref="MailController"/> class.
        /// </summary>
        /// <param name="configuration">The application configuration.</param>
        /// <param name="mapper">The AutoMapper instance for object mapping.</param>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="rabbitMqService">The RabbitMQ service for mail notifications.</param>
        public MailController(IConfiguration configuration, IMapper mapper, DbContext dbContext, RabbitMqService rabbitMqService)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
        }

        /// <summary>
        /// Retrieves a paginated list of mail messages for a specific user.
        /// Returns mail summaries with pagination support for the user's mailbox.
        /// </summary>
        /// <param name="user">The user ID to retrieve mail for.</param>
        /// <param name="offset">The starting position for pagination.</param>
        /// <param name="count">The number of mail items to retrieve.</param>
        /// <returns>A response containing the mail summary list or error details.</returns>
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

        /// <summary>
        /// Retrieves a specific mail message by user and mail ID.
        /// Returns the full mail content and updates unread count for the user.
        /// </summary>
        /// <param name="user">The user ID who owns the mail.</param>
        /// <param name="id">The unique identifier of the mail message.</param>
        /// <returns>A response containing the mail content and unread count or error details.</returns>
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

        /// <summary>
        /// Handles mail writing requests to send messages between users.
        /// Creates a new mail message and publishes notification via RabbitMQ.
        /// </summary>
        /// <param name="request">The mail writing request containing recipient, sender UID, title, and content.</param>
        /// <returns>A response with the created mail information and unread count or error details.</returns>
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

        /// <summary>
        /// Handles mail deletion requests for removing messages from user mailboxes.
        /// Deletes the specified mail and returns updated unread count.
        /// </summary>
        /// <param name="request">The mail deletion request containing user ID and mail ID.</param>
        /// <returns>A response with updated unread count or error details.</returns>
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

        /// <summary>
        /// Retrieves all active (non-expired and non-deleted) system mails.
        /// Returns system mails that are currently valid for distribution to users.
        /// </summary>
        /// <returns>A response containing the list of active system mails or error details.</returns>
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

        /// <summary>
        /// Creates a new system mail and publishes notification via RabbitMQ.
        /// Invalidates Redis cache to ensure all game servers fetch the latest system mails.
        /// </summary>
        /// <param name="request">The system mail writing request containing sender, title, contents, and optional expiration date.</param>
        /// <returns>A response with the created system mail information or error details.</returns>
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