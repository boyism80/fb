using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Threading.Tasks;
using Fb.Model;
using Http.Model;
using Http.Model.Redis;
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
            var globalTask = _dbContext.StoragePendingBox.Get(null);
            var personalTask = _dbContext.StoragePendingBox.Get(user);
            var rewardMarksTask = _dbContext.StorageRewardMark.Get(user);

            await Task.WhenAll(globalTask, personalTask, rewardMarksTask);

            var now = DateTime.Now;
            var processedIds = (await rewardMarksTask)
                .Where(mark => !mark.Deleted)
                .Select(mark => mark.PendingId)
                .ToHashSet();

            return (await globalTask)
                .Concat(await personalTask)
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

        public async Task<StoragePendingBox> CreatePendingAsync(string message, string? userName, DateTime? expiredDate)
        {
            if (string.IsNullOrWhiteSpace(message))
                throw new ArgumentException("Message is required", nameof(message));

            uint? userId = null;
            if (string.IsNullOrWhiteSpace(userName) == false)
            {
                var id = await _dbContext.Character.GetCharacterId(userName.Trim());
                if (id.HasValue == false)
                    throw new KeyNotFoundException($"Character '{userName}' not found.");

                userId = id.Value;
            }

            var pending = new StoragePendingBox
            {
                Id = GeneratePendingId(),
                User = userId,
                Message = message.Trim(),
                Attachments = new List<Dsl>(),
                ExpiredDate = expiredDate
            };

            return await AddPendingAsync(pending);
        }

        public async Task<StoragePendingBox> AddPendingAsync(StoragePendingBox pending)
        {
            var now = DateTime.Now;
            pending.CreatedDate = now;
            pending.UpdatedDate = now;
            pending.Deleted = false;
            pending.Attachments ??= new List<Dsl>();

            _dbContext.StoragePendingBox.Set(pending);
            await _dbContext.SaveChangesAsync();

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

        private Task NotifyGlobalPendingAsync()
        {
            _rabbitMqService.Publish(BuildPendingResponse(null, Enumerable.Empty<StoragePendingBox>()), "amq.direct", "fb.global");
            return Task.CompletedTask;
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
                Message = box.Message ?? string.Empty,
                Attachments = JsonConvert.SerializeObject(box.Attachments ?? new List<Dsl>()),
                ExpiredDate = box.ExpiredDate?.ToString("yyyy-MM-dd HH:mm:ss")
            };
        }

        private static ulong GeneratePendingId()
        {
            Span<byte> buffer = stackalloc byte[8];
            RandomNumberGenerator.Fill(buffer);
            var value = BitConverter.ToUInt64(buffer);
            return value == 0 ? 1UL : value;
        }
    }
}

