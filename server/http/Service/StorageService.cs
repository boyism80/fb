using AutoMapper;
using Fb.Model.EnumValue;
using Http.Model;
using Protocol = fb.protocol._internal;
using Response = fb.protocol._internal.response;

namespace Http.Service
{
    public class StorageService
    {
        private readonly DbContext _dbContext;
        private readonly LogService _logService;
        private readonly RabbitMqService _rabbitMqService;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly IMapper _mapper;

        public StorageService(DbContext dbContext,
            LogService logService,
            RabbitMqService rabbitMqService,
            RedisDistributedLockService distributedLock,
            IMapper mapper)
        {
            _dbContext = dbContext;
            _logService = logService;
            _rabbitMqService = rabbitMqService;
            _distributedLock = distributedLock;
            _mapper = mapper;
        }

        private static string OpsLockKey(uint user) => $"fb:lock:storage:box:ops:{user}";

        /// <summary>
        /// Personal storage box: allocate id, Set (Redis first + write-back), MQ notify.
        /// Caller must resolve name to uid before calling.
        /// </summary>
        public async Task<StorageBox> CreateStorageBoxAsync(uint world,
            uint userId,
            string title,
            string message,
            List<Fb.Model.Dsl> attachments = null,
            DateTime? expiredDate = null,
            string externalRef = null,
            uint host = 0)
        {
            if (userId == 0)
                throw new ArgumentException("User id is required", nameof(userId));

            if (string.IsNullOrWhiteSpace(title))
                throw new ArgumentException("Title is required", nameof(title));

            if (string.IsNullOrWhiteSpace(message))
                throw new ArgumentException("Message is required", nameof(message));

            var attachmentsList = attachments ?? new List<Fb.Model.Dsl>();
            if (attachmentsList.Count == 0)
                throw new ArgumentException("At least one attachment is required (item, money, or exp)", nameof(attachments));

            await using var _ = await _distributedLock.Lock(world, OpsLockKey(userId));

            var now = DateTime.Now;
            var id = await _dbContext.StorageBox.AllocateNextIdAsync(world, userId);
            var box = new StorageBox
            {
                User               = userId,
                Id                 = id,
                SystemStorageBoxId = null,
                Title              = title.Trim(),
                Message            = message.Trim(),
                Attachments        = attachmentsList,
                Received           = false,
                ExpiredDate        = expiredDate,
                CreatedDate        = now,
                UpdatedDate        = now
            };

            _dbContext.StorageBox.Set(world, box);
            await _dbContext.StorageBox.SaveChangesAsync();

            await _logService.WriteAsync("storage_box_create", new
            {
                box_id       = box.Id,
                user_id      = box.User,
                external_ref = externalRef,
                expired_date = expiredDate?.ToString("yyyy-MM-dd HH:mm:ss")
            });

            await PublishWriteAsync(world, box, host);
            return box;
        }

        /// <summary>
        /// Broadcast system template only (world-global). No per-user payload.
        /// </summary>
        public async Task<SystemStorageBox> CreateSystemStorageAsync(uint world,
            string title,
            string message,
            DateTime? expiredDate = null,
            List<Fb.Model.Dsl> attachments = null,
            string externalRef = null)
        {
            if (string.IsNullOrWhiteSpace(title))
                throw new ArgumentException("Title is required", nameof(title));

            if (string.IsNullOrWhiteSpace(message))
                throw new ArgumentException("Message is required", nameof(message));

            var attachmentsList = attachments ?? new List<Fb.Model.Dsl>();
            if (attachmentsList.Count == 0)
                throw new ArgumentException("At least one attachment is required (item, money, or exp)", nameof(attachments));

            var box = await _dbContext.SystemStorageBox.Write(world,
                title.Trim(),
                message.Trim(),
                attachmentsList,
                expiredDate,
                externalRef);

            await _logService.WriteAsync("system_storage_box_create", new
            {
                box_id       = box.Id,
                external_ref = box.ExternalRef,
                expired_date = expiredDate?.ToString("yyyy-MM-dd HH:mm:ss")
            });

            return box;
        }

        public async Task<List<StorageBox>> DeliverSystemStorageAsync(uint world,
            uint systemStorageBoxId,
            IReadOnlyList<uint> users,
            uint host = 0)
        {
            var systemBox = await _dbContext.SystemStorageBox.Get(world, systemStorageBoxId);
            if (systemBox == null)
                throw new KeyNotFoundException($"System storage box {systemStorageBoxId} not found");

            var now = DateTime.Now;
            if (systemBox.ExpiredDate.HasValue && systemBox.ExpiredDate.Value <= now)
                return new List<StorageBox>();

            var distinctUsers = (users ?? Array.Empty<uint>()).Where(u => u != 0).Distinct().ToArray();
            var written = new List<StorageBox>();

            foreach (var userId in distinctUsers)
            {
                await using var _ = await _distributedLock.Lock(world, OpsLockKey(userId));

                var existing = await _dbContext.StorageBox.Get(world, userId);
                if (existing.Any(b => b.SystemStorageBoxId == systemStorageBoxId))
                    continue;

                var id = await _dbContext.StorageBox.AllocateNextIdAsync(world, userId);
                var box = new StorageBox
                {
                    User               = userId,
                    Id                 = id,
                    SystemStorageBoxId = systemStorageBoxId,
                    Title              = systemBox.Title,
                    Message            = systemBox.Message,
                    Attachments        = systemBox.Attachments ?? new List<Fb.Model.Dsl>(),
                    Received           = false,
                    ExpiredDate        = systemBox.ExpiredDate,
                    CreatedDate        = now,
                    UpdatedDate        = now
                };

                _dbContext.StorageBox.Set(world, box);
                written.Add(box);
            }

            if (written.Count > 0)
            {
                await _dbContext.StorageBox.SaveChangesAsync();
                await PublishDeliverAsync(world, written, host);
            }

            return written;
        }

        public async Task<bool> ClaimAsync(uint world, uint userId, uint id)
        {
            await using var _ = await _distributedLock.Lock(world, OpsLockKey(userId));

            var box = await _dbContext.StorageBox.Get(world, userId, id);
            if (box == null)
                return false;

            var now = DateTime.Now;
            if (box.Received)
                return false;

            if (box.ExpiredDate.HasValue && box.ExpiredDate.Value <= now)
                return false;

            box.Received = true;
            box.UpdatedDate = now;
            _dbContext.StorageBox.Set(world, box);
            await _dbContext.StorageBox.SaveChangesAsync();
            return true;
        }

        public async Task<bool> UnclaimAsync(uint world, uint userId, uint id)
        {
            await using var _ = await _distributedLock.Lock(world, OpsLockKey(userId));

            var box = await _dbContext.StorageBox.Get(world, userId, id);
            if (box == null)
                return false;

            if (!box.Received)
                return false;

            box.Received = false;
            box.UpdatedDate = DateTime.Now;
            _dbContext.StorageBox.Set(world, box);
            await _dbContext.StorageBox.SaveChangesAsync();
            return true;
        }

        private async Task PublishWriteAsync(uint world, StorageBox box, uint host)
        {
            var response = new Response.WriteStorageBox
            {
                Box   = _mapper.Map<Protocol.StorageBox>(box),
                Host  = host,
                Error = (uint)ErrorCode.None
            };
            await _rabbitMqService.PublishFanoutAsync(response, "storage", world);
        }

        private async Task PublishDeliverAsync(uint world, List<StorageBox> boxes, uint host)
        {
            var entries = boxes.Select(box => new Protocol.StorageWriteEntry
            {
                User = box.User,
                Box  = _mapper.Map<Protocol.StorageBox>(box)
            }).ToList();

            var response = new Response.DeliverSystemStorage
            {
                Entries = entries,
                Host    = host,
                Error   = (uint)ErrorCode.None
            };
            await _rabbitMqService.PublishFanoutAsync(response, "storage", world);
        }
    }
}
