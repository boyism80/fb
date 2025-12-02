using Fb.Model;
using Http.Model;
using Newtonsoft.Json;
using Protocol = fb.protocol._internal;
using Response = fb.protocol._internal.response;

namespace Http.Service
{
    /// <summary>
    /// Provides storage-related aggregation logic for controllers, admin tools, and services.
    /// </summary>
    public class StorageService
    {
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly SessionService _sessionService;
        private readonly LogService? _logService;

        public StorageService(DbContext dbContext,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            LogService? logService = null)
        {
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
            _logService = logService;
        }

        public async Task<List<StoragePendingBox>> GetPendingForUserAsync(uint user)
        {
            var personalTask = _dbContext.StoragePendingBox.Get(user);
            var rewardMarksTask = _dbContext.StorageRewardMark.Get(user);

            await Task.WhenAll(personalTask, rewardMarksTask);

            var now = DateTime.Now;
            var processedIds = (await rewardMarksTask)
                .Where(mark => !mark.Deleted)
                .Select(mark => mark.PendingId)
                .ToHashSet();

            return (await personalTask)
                .Where(box => !box.Deleted)
                .Where(box => box.ExpiredDate == null || box.ExpiredDate >= now)
                .Where(box => !processedIds.Contains(box.Id))
                .ToList();
        }

        public async Task<List<StoragePendingBox>> GetGlobalPendingAsync()
        {
            var pending = await _dbContext.StoragePendingBox.Get(null);
            var now = DateTime.Now;

            return pending
                .Where(box => !box.Deleted)
                .Where(box => box.ExpiredDate == null || box.ExpiredDate >= now)
                .ToList();
        }

        public async Task<StoragePendingBox> CreatePendingAsync(string title, string message, string userName = null, DateTime? expiredDate = null, List<Dsl> attachments = null)
        {
            if (string.IsNullOrWhiteSpace(title))
                throw new ArgumentException("Title is required", nameof(title));

            if (string.IsNullOrWhiteSpace(message))
                throw new ArgumentException("Message is required", nameof(message));

            var attachmentsList = attachments ?? new List<Dsl>();
            if (attachmentsList.Count == 0)
                throw new ArgumentException("At least one attachment is required (item, money, or exp)", nameof(attachments));

            var normalizedTitle = title.Trim();
            var normalizedMessage = message.Trim();

            uint? userId = null;
            if (string.IsNullOrWhiteSpace(userName) == false)
            {
                var id = await _dbContext.Character.GetCharacterId(userName.Trim());
                if (id.HasValue == false)
                    throw new KeyNotFoundException($"Character '{userName}' not found.");

                userId = id.Value;
            }

            // Generate UUID for pending ID (as string, MySql.Escape() will convert to BINARY(16))
            var pendingId = Guid.NewGuid().ToString();

            // Create pending box entity
            var pending = new StoragePendingBox
            {
                Id = pendingId,
                User = userId,
                Title = normalizedTitle,
                Message = normalizedMessage,
                Attachments = attachmentsList,
                ExpiredDate = expiredDate,
                Deleted = false,
                CreatedDate = DateTime.Now,
                UpdatedDate = DateTime.Now
            };

            // Save using Repository (handles Redis caching and DB write-back automatically)
            _dbContext.StoragePendingBox.Set(pending);
            await _dbContext.SaveChangesAsync();

            // Log storage pending creation event
            _logService?.Write("storage_pending_create", new
            {
                pending_id = pending.Id,
                user_id = pending.User,
                user_name = userName,
                is_global = !pending.User.HasValue,
                expired_date = expiredDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? null
            });

            if (pending.User.HasValue)
                await NotifyPersonalPendingAsync(pending.User.Value);
            else
                await NotifyGlobalPendingAsync();

            return pending;
        }

        private async Task NotifyPersonalPendingAsync(uint user)
        {
            var pending = await GetPendingForUserAsync(user);
            if (pending.Count == 0)
                return;

            var character = await _dbContext.Character.Get(user);
            if (character == null || string.IsNullOrWhiteSpace(character.Name))
                return;

            var session = await _sessionService.Get(character.Name);
            if (session == null)
                return;

            var response = BuildPendingResponse(user, pending);
            _rabbitMqService.Publish(response, "amq.direct", $"fb.game.{session.Host}");
        }

        private async Task NotifyGlobalPendingAsync()
        {
            var pending = await GetGlobalPendingAsync();
            if (pending.Count == 0)
                return;

            var response = BuildPendingResponse(null, pending);
            _rabbitMqService.Publish(response, "amq.direct", "fb.global");
        }

        private Response.GetStoragePending BuildPendingResponse(uint? user, IEnumerable<StoragePendingBox> pending)
        {
            return new Response.GetStoragePending
            {
                User = user,
                Pending = pending.Select(ToProtocol).ToList(),
                Error = 0
            };
        }

        private static Protocol.StoragePendingBox ToProtocol(StoragePendingBox box)
        {
            return new Protocol.StoragePendingBox
            {
                Id = box.Id,
                User = box.User,
                Title = box.Title ?? string.Empty,
                Message = box.Message ?? string.Empty,
                Attachments = JsonConvert.SerializeObject(box.Attachments ?? new List<Dsl>()),
                ExpiredDate = box.ExpiredDate?.ToString("yyyy-MM-dd HH:mm:ss")
            };
        }

    }
}

