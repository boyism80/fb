using AutoMapper;
using Fb.Model.EnumValue;
using Http.Model;
using Protocol = fb.protocol.@internal;
using Response = fb.protocol.@internal.response;

namespace Http.Service
{
    public class StorageService
    {
        private readonly DbContext _dbContext;
        private readonly LogService _logService;
        private readonly RabbitMqService _rabbitMqService;
        private readonly IMapper _mapper;

        public StorageService(DbContext dbContext,
            LogService logService,
            RabbitMqService rabbitMqService,
            IMapper mapper)
        {
            _dbContext = dbContext;
            _logService = logService;
            _rabbitMqService = rabbitMqService;
            _mapper = mapper;
        }

        public static string ExternalRefLockKey(string externalRef) => $"fb:lock:storage:ref:{externalRef}";

        public async Task<SystemStorageBox> CreateSystemStorageAsync(uint world,
            string title,
            string message,
            DateTime? expiredDate = null,
            List<Fb.Model.Dsl> attachments = null,
            string externalRef = null,
            DateTime? reserveDate = null,
            bool includeNewCharacters = false,
            uint? user = null)
        {
            if (string.IsNullOrWhiteSpace(title))
            {
                throw new ArgumentException("Title is required", nameof(title));
            }

            if (string.IsNullOrWhiteSpace(message))
            {
                throw new ArgumentException("Message is required", nameof(message));
            }

            var attachmentsList = attachments ?? new List<Fb.Model.Dsl>();
            if (attachmentsList.Count == 0)
            {
                throw new ArgumentException("At least one attachment is required (item, money, or exp)", nameof(attachments));
            }

            if (user == 0)
            {
                throw new ArgumentException("User id must not be 0", nameof(user));
            }

            var box = await _dbContext.SystemStorageBox.Write(world,
                title.Trim(),
                message.Trim(),
                attachmentsList,
                reserveDate,
                expiredDate,
                includeNewCharacters,
                externalRef,
                user);

            await _logService.WriteAsync("system_storage_box_create", new
            {
                box_id = box.Id,
                user_id = box.User,
                external_ref = box.ExternalRef,
                reserve_date = reserveDate?.ToString("yyyy-MM-dd HH:mm:ss"),
                expired_date = expiredDate?.ToString("yyyy-MM-dd HH:mm:ss")
            });

            if (box.User != null)
            {
                var response = new Response.WriteSystemStorageBox
                {
                    Box = _mapper.Map<Protocol.SystemStorageBox>(box),
                    Error = (uint)ErrorCode.None
                };
                await _rabbitMqService.NotifySavedAsync(response, "storage", world);
            }

            return box;
        }
    }
}
