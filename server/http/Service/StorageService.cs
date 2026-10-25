using AutoMapper;
using Fb.Model.EnumValue;
using Http.Model;
using Microsoft.Extensions.Logging;
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
        private readonly ILogger<StorageService> _logger;

        public StorageService(DbContext dbContext,
            LogService logService,
            RabbitMqService rabbitMqService,
            IMapper mapper,
            ILogger<StorageService> logger)
        {
            _dbContext = dbContext;
            _logService = logService;
            _rabbitMqService = rabbitMqService;
            _mapper = mapper;
            _logger = logger;
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

        public async Task SettleLegacyRequestsAsync(uint world, uint user)
        {
            var unsettled = (await _dbContext.UnsettledRequest.Get(world, user))
                .Where(x => x.Type is (byte)UnsettledType.StorageClaim or (byte)UnsettledType.MarketList or (byte)UnsettledType.MarketPurchase)
                .ToList();
            if (unsettled.Count == 0)
            {
                return;
            }

            var boxes = (await _dbContext.StorageBox.Get(world, user)).ToList();
            var settled = new List<UnsettledRequest>();
            foreach (var request in unsettled)
            {
                if (request.Type == (byte)UnsettledType.StorageClaim)
                {
                    var box = boxes.FirstOrDefault(x => x.Id == request.Target);
                    if (box != null && box.Received && box.ClaimId == request.Id)
                    {
                        box.Received = false;
                        box.ClaimId = null;
                        _dbContext.StorageBox.Set(world, box);
                    }

                    settled.Add(request);
                }
                else
                {
                    var prefix = request.Type == (byte)UnsettledType.MarketList ? "marketplace:list" : "marketplace:purchase";
                    var externalRef = $"{prefix}:{request.Id}";
                    if (boxes.Any(x => x.ExternalRef == externalRef))
                    {
                        settled.Add(request);
                    }
                }
            }

            if (settled.Count == 0)
            {
                return;
            }

            _dbContext.UnsettledRequest.Delete(world, settled);
            await _dbContext.SaveChangesAsync();
            _logger.LogInformation("Settled {Count} legacy storage requests of user {User}", settled.Count, user);
        }
    }
}
