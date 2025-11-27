using Dapper;
using Fb.Model;
using Http.Model;
using Newtonsoft.Json;
using System.Data;
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

        public StorageService(DbContext dbContext,
            RabbitMqService rabbitMqService,
            SessionService sessionService)
        {
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
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
                .Where(box => box.ExpiredDate == null || box.ExpiredDate <= now)
                .Where(box => !processedIds.Contains(box.Id))
                .ToList();
        }

        public async Task<List<StoragePendingBox>> GetGlobalPendingAsync()
        {
            var pending = await _dbContext.StoragePendingBox.Get(null);
            var now = DateTime.Now;

            return pending
                .Where(box => !box.Deleted)
                .Where(box => box.ExpiredDate == null || box.ExpiredDate <= now)
                .ToList();
        }

        private async Task<ulong> GetNextPendingIdAsync()
        {
            // Sequence is stored in global DB (single global sequence)
            await using var globalConn = _dbContext.Connection(-1);
            await globalConn.OpenAsync();

            var result = await globalConn.QueryFirstOrDefaultAsync<dynamic>(
                "USP_STORAGE_PENDING_GET_NEXT_ID",
                commandType: CommandType.StoredProcedure);

            if (result == null || result.RESULT != 1 || result.id == null)
                throw new Exception("Failed to get next pending ID from sequence");

            return (ulong)result.id;
        }

        /// <summary>
        /// Creates a new storage pending reward with optional attachments.
        /// </summary>
        /// <param name="title">Short title used by clients when listing rewards.</param>
        /// <param name="message">Detailed message displayed with the reward.</param>
        /// <param name="userName">The user name for personal rewards, or null for global rewards.</param>
        /// <param name="expiredDate">Optional expiration date for the reward.</param>
        /// <param name="attachments">Optional list of DSL attachments (items, money, exp).</param>
        /// <returns>The created storage pending box.</returns>
        public async Task<StoragePendingBox> CreatePendingAsync(string title, string message, string? userName, DateTime? expiredDate, List<Dsl>? attachments = null)
        {
            if (string.IsNullOrWhiteSpace(title))
                throw new ArgumentException("Title is required", nameof(title));

            if (string.IsNullOrWhiteSpace(message))
                throw new ArgumentException("Message is required", nameof(message));

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

            // Get ID from global sequence
            var pendingId = await GetNextPendingIdAsync();

            var attachmentsList = attachments ?? new List<Dsl>();
            var attachmentsJson = JsonConvert.SerializeObject(attachmentsList);

            // Determine shard: use -1 for global (userId is null), otherwise use user ID
            await using var conn = userId != null ? _dbContext.Connection(userId.Value) : _dbContext.Connection(-1);
            await conn.OpenAsync();

            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("p_id", pendingId);
            dynamicParams.Add("p_user", userId);
            dynamicParams.Add("p_title", normalizedTitle);
            dynamicParams.Add("p_message", normalizedMessage);
            dynamicParams.Add("p_attachments", attachmentsJson);
            dynamicParams.Add("p_expired_date", expiredDate);

            await using var reader = await conn.QueryMultipleAsync(
                "USP_STORAGE_PENDING_ADD",
                dynamicParams,
                commandType: CommandType.StoredProcedure);

            var result = await reader.ReadFirstOrDefaultAsync<dynamic>();
            if (result == null || result.RESULT != 1 || result.id == null)
                throw new Exception("Failed to create pending storage box");

            var pending = await reader.ReadFirstOrDefaultAsync<StoragePendingBox>();
            if (pending == null)
                throw new Exception("Failed to retrieve created pending storage box");

            // Set metadata
            pending.Deleted = false;
            pending.Title = normalizedTitle;
            pending.Message = normalizedMessage;
            pending.Attachments = attachmentsList;

            // Cache and notify
            _dbContext.StoragePendingBox.Set(pending);
            await _dbContext.SaveChangesAsync();

            if (pending.User.HasValue)
                await NotifyPersonalPendingAsync(pending.User.Value);
            else
                await NotifyGlobalPendingAsync();

            return pending;
        }

        public async Task<StoragePendingBox> AddPendingAsync(StoragePendingBox pending)
        {
            if (pending == null)
                throw new ArgumentNullException(nameof(pending));

            pending.Title = (pending.Title ?? string.Empty).Trim();

            // Get ID from global sequence
            var pendingId = await GetNextPendingIdAsync();

            var attachmentsJson = JsonConvert.SerializeObject(pending.Attachments ?? new List<Dsl>());

            // Determine shard: use -1 for global (user is null), otherwise use user ID
            await using var conn = pending.User != null ? _dbContext.Connection(pending.User.Value) : _dbContext.Connection(-1);
            await conn.OpenAsync();

            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("p_id", pendingId);
            dynamicParams.Add("p_user", pending.User);
            dynamicParams.Add("p_title", pending.Title);
            dynamicParams.Add("p_message", pending.Message);
            dynamicParams.Add("p_attachments", attachmentsJson);
            dynamicParams.Add("p_expired_date", pending.ExpiredDate);

            await using var reader = await conn.QueryMultipleAsync(
                "USP_STORAGE_PENDING_ADD",
                dynamicParams,
                commandType: CommandType.StoredProcedure);

            var result = await reader.ReadFirstOrDefaultAsync<dynamic>();
            if (result == null || result.RESULT != 1 || result.id == null)
                throw new Exception("Failed to add pending storage box");

            var created = await reader.ReadFirstOrDefaultAsync<StoragePendingBox>();
            if (created == null)
                throw new Exception("Failed to retrieve created pending storage box");

            // Set metadata
            created.Deleted = false;
            created.Title = pending.Title ?? string.Empty;
            created.Attachments = pending.Attachments ?? new List<Dsl>();

            // Cache and notify
            _dbContext.StoragePendingBox.Set(created);
            await _dbContext.SaveChangesAsync();

            if (created.User.HasValue)
                await NotifyPersonalPendingAsync(created.User.Value);
            else
                await NotifyGlobalPendingAsync();

            return created;
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

