using AutoMapper;
using Dapper;
using Fb.Model.EnumValue;
using Http.Model;
using Http.Util;
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
        private readonly RedisDistributedLockService _distributedLock;
        private readonly IMapper _mapper;
        private readonly ILogger<StorageService> _logger;

        public StorageService(DbContext dbContext,
            LogService logService,
            RabbitMqService rabbitMqService,
            RedisDistributedLockService distributedLock,
            IMapper mapper,
            ILogger<StorageService> logger)
        {
            _dbContext = dbContext;
            _logService = logService;
            _rabbitMqService = rabbitMqService;
            _distributedLock = distributedLock;
            _mapper = mapper;
            _logger = logger;
        }

        private static string OpsLockKey(uint user) => $"fb:lock:storage:box:ops:{user}";

        // Shorter than the game's 5 second POST timeout, so a claim that waits on the lock fails before the game
        // gives up on the response.
        private static readonly TimeSpan ClaimLockTimeout = TimeSpan.FromSeconds(3);

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
            {
                throw new ArgumentException("User id is required", nameof(userId));
            }

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

            StorageBox box;
            await using (await _distributedLock.Lock(world, OpsLockKey(userId)))
            {
                // A retried write with the same external ref returns the box it already created. The first
                // attempt's notice may have been lost, so an unreceived box is published again.
                if (externalRef != null)
                {
                    var existing = (await _dbContext.StorageBox.Get(world, userId)).FirstOrDefault(b => b.ExternalRef == externalRef);
                    if (existing != null)
                    {
                        if (existing.Received == false)
                        {
                            await PublishWriteAsync(world, existing, host);
                        }

                        return existing;
                    }
                }

                var now = DateTime.UtcNow;
                var id = await _dbContext.StorageBox.AllocateNextIdAsync(world, userId);
                box = new StorageBox
                {
                    User = userId,
                    Id = id,
                    SystemStorageBoxId = null,
                    ExternalRef = externalRef,
                    Title = title.Trim(),
                    Message = message.Trim(),
                    Attachments = attachmentsList,
                    Received = false,
                    ExpiredDate = expiredDate,
                    CreatedDate = now,
                    UpdatedDate = now
                };

                _dbContext.StorageBox.Set(world, box);
                await _dbContext.StorageBox.SaveChangesAsync();
            }

            await _logService.WriteAsync("storage_box_create", new
            {
                box_id = box.Id,
                user_id = box.User,
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
            string externalRef = null,
            DateTime? reserveDate = null,
            bool includeNewCharacters = false)
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

            var box = await _dbContext.SystemStorageBox.Write(world,
                title.Trim(),
                message.Trim(),
                attachmentsList,
                reserveDate,
                expiredDate,
                includeNewCharacters,
                externalRef);

            await _logService.WriteAsync("system_storage_box_create", new
            {
                box_id = box.Id,
                external_ref = box.ExternalRef,
                reserve_date = reserveDate?.ToString("yyyy-MM-dd HH:mm:ss"),
                expired_date = expiredDate?.ToString("yyyy-MM-dd HH:mm:ss")
            });

            return box;
        }

        public async Task<List<StorageBox>> DistributeSystemStorageAsync(uint world,
            uint systemStorageBoxId,
            IReadOnlyList<uint> users,
            uint host = 0)
        {
            var systemBox = await _dbContext.SystemStorageBox.Get(world, systemStorageBoxId);
            if (systemBox == null)
            {
                throw new KeyNotFoundException($"System storage box {systemStorageBoxId} not found");
            }

            var now = DateTime.UtcNow;
            if (systemBox.Active(now) == false)
            {
                return new List<StorageBox>();
            }

            var distinctUsers = (users ?? Array.Empty<uint>()).Where(u => u != 0).Distinct().ToArray();
            var written = new List<StorageBox>();

            // Copies committed before a later user fails must still be published.
            try
            {
                foreach (var userId in distinctUsers)
                {
                    await using var _ = await _distributedLock.Lock(world, OpsLockKey(userId));

                    var existing = (await _dbContext.StorageBox.Get(world, userId)).FirstOrDefault(b => b.SystemStorageBoxId == systemStorageBoxId);
                    if (existing != null)
                    {
                        if (existing.Received == false)
                        {
                            written.Add(existing);
                        }

                        continue;
                    }

                    var id = await _dbContext.StorageBox.AllocateNextIdAsync(world, userId);
                    var box = new StorageBox
                    {
                        User = userId,
                        Id = id,
                        SystemStorageBoxId = systemStorageBoxId,
                        Title = systemBox.Title,
                        Message = systemBox.Message,
                        Attachments = systemBox.Attachments ?? new List<Fb.Model.Dsl>(),
                        Received = false,
                        ExpiredDate = systemBox.ExpiredDate,
                        CreatedDate = now,
                        UpdatedDate = now
                    };

                    _dbContext.StorageBox.Set(world, box);
                    await _dbContext.StorageBox.SaveChangesAsync();
                    written.Add(box);
                }
            }
            finally
            {
                if (written.Count > 0)
                {
                    await PublishDistributedAsync(world, written, host);
                }
            }

            return written;
        }

        // The game retries with the same claim id when a response is lost, so a repeat of the claim that already
        // holds the box succeeds instead of reporting the box as taken.
        public async Task<ErrorCode> ClaimAsync(uint world, uint userId, uint id, ulong claimId)
        {
            await using var _ = await _distributedLock.Lock(world, OpsLockKey(userId), ClaimLockTimeout);

            var box = await _dbContext.StorageBox.Get(world, userId, id);
            if (box == null)
            {
                return ErrorCode.StorageBoxNotFound;
            }

            if (box.Received)
            {
                return claimId != 0 && box.ClaimId == claimId ? ErrorCode.None : ErrorCode.StorageBoxAlreadyReceived;
            }

            // A claim resent after an outage is judged by when the game issued it, not by when it arrived.
            if (box.ExpiredDate.HasValue && box.ExpiredDate.Value <= SnowflakeId.Time(claimId))
            {
                return ErrorCode.StorageBoxExpired;
            }

            var now = DateTime.UtcNow;
            box.Received = true;
            box.ClaimId = claimId;
            box.UpdatedDate = now;
            _dbContext.StorageBox.Set(world, box);
            await _dbContext.StorageBox.SaveChangesAsync();
            return ErrorCode.None;
        }

        public async Task<ErrorCode> UnclaimAsync(uint world, uint userId, uint id, ulong claimId)
        {
            await using var _ = await _distributedLock.Lock(world, OpsLockKey(userId), ClaimLockTimeout);

            var box = await _dbContext.StorageBox.Get(world, userId, id);
            if (box == null)
            {
                return ErrorCode.StorageBoxNotFound;
            }

            if (!box.Received)
            {
                return ErrorCode.None;
            }

            if (box.ClaimId != claimId)
            {
                return ErrorCode.StorageBoxAlreadyReceived;
            }

            box.Received = false;
            box.ClaimId = null;
            box.UpdatedDate = DateTime.UtcNow;
            _dbContext.StorageBox.Set(world, box);
            await _dbContext.StorageBox.SaveChangesAsync();
            return ErrorCode.None;
        }

        public async Task<int> CleanupBoxesAsync(uint world, int pageSize)
        {
            var deleted = 0;
            for (var db = 0; db < _dbContext.GetShardDbSize(world); db++)
            {
                var archived = await ArchivedSystemBoxesAsync(world, db);
                var archivedIds = archived.ToArray();
                var afterUser = 0u;
                List<uint> users;
                do
                {
                    await using (var conn = _dbContext.GetDataConnection(world, db))
                    {
                        users = (await conn.QueryAsync<uint>(
                            """
                            SELECT DISTINCT `user` FROM `storage_box`
                            WHERE `deleted` = 0 AND `user` > @afterUser AND (
                                (`received` = 1 AND `system_storage_box_id` IS NULL)
                                OR (`received` = 0 AND `expired_date` IS NOT NULL AND `expired_date` <= UTC_TIMESTAMP(3))
                                OR `system_storage_box_id` IN @archivedIds)
                            ORDER BY `user`
                            LIMIT @pageSize
                            """,
                            new { afterUser, archivedIds, pageSize })).ToList();
                    }

                    foreach (var user in users)
                    {
                        deleted += await DeleteSettledBoxesAsync(world, user, archived);
                        afterUser = user;
                    }
                } while (users.Count == pageSize);
            }

            return deleted;
        }

        private async Task<HashSet<uint>> ArchivedSystemBoxesAsync(uint world, int db)
        {
            var archived = new HashSet<uint>();
            uint[] ids;
            await using (var conn = _dbContext.GetDataConnection(world, db))
            {
                ids = (await conn.QueryAsync<uint>(
                    """
                    SELECT DISTINCT `system_storage_box_id` FROM `storage_box`
                    WHERE `system_storage_box_id` IS NOT NULL AND `deleted` = 0
                    """)).ToArray();
            }

            if (ids.Length == 0)
            {
                return archived;
            }

            await using var global = _dbContext.GetGlobalConnection(world);
            foreach (var chunk in ids.Chunk(1000))
            {
                archived.UnionWith(await global.QueryAsync<uint>(
                    "SELECT `id` FROM `system_storage_box_archive` WHERE `id` IN @chunk", new { chunk }));
            }

            return archived;
        }

        private async Task<int> DeleteSettledBoxesAsync(uint world, uint user, IReadOnlySet<uint> archivedSystemBoxes)
        {
            await using var opsLock = await _distributedLock.Lock(world, OpsLockKey(user));
            await using var saveLock = await _distributedLock.Lock(world, Character.SaveLockKey(user));

            var character = await _dbContext.Character.Get(world, user);
            var snapshotTime = character?.SnapshotTime;
            var unsettled = await _dbContext.UnsettledRequest.Get(world, user);
            var now = DateTime.UtcNow;
            var settled = new List<StorageBox>();
            foreach (var box in await _dbContext.StorageBox.Get(world, user))
            {
                var archived = box.SystemStorageBoxId is { } systemId && archivedSystemBoxes.Contains(systemId);
                if (box.Received)
                {
                    if (snapshotTime == null || (box.SystemStorageBoxId != null && archived == false))
                    {
                        continue;
                    }

                    var claimedAt = box.ClaimId is > 0
                        ? SnowflakeId.Time(box.ClaimId.Value)
                        : DateTime.SpecifyKind(box.UpdatedDate, DateTimeKind.Utc);
                    var savedAfterClaim = snapshotTime.Value > new DateTimeOffset(claimedAt).ToUnixTimeMilliseconds();
                    var claimPending = box.ClaimId is > 0
                        ? unsettled.Any(request => request.Id == box.ClaimId.Value)
                        : unsettled.Any(request => request.Type == (byte)UnsettledType.StorageClaim && request.Target == box.Id);
                    if (savedAfterClaim && claimPending == false)
                    {
                        settled.Add(box);
                    }
                }
                else
                {
                    var expired = box.ExpiredDate.HasValue && box.ExpiredDate.Value <= now;
                    if (expired == false && archived == false)
                    {
                        continue;
                    }

                    var claimPending = unsettled.Any(request => request.Type == (byte)UnsettledType.StorageClaim && request.Target == box.Id);
                    if (claimPending == false)
                    {
                        settled.Add(box);
                    }
                }
            }

            var refs = settled
                .Select(box => box.ExternalRef)
                .Where(externalRef => string.IsNullOrEmpty(externalRef) == false)
                .ToArray();
            var parcels = new HashSet<string>();
            if (refs.Length > 0)
            {
                await using var unified = _dbContext.GetUnifiedConnection();
                parcels.UnionWith(await unified.QueryAsync<string>(
                    "SELECT `external_ref` FROM `marketplace_parcel` WHERE `external_ref` IN @refs",
                    new { refs }));
            }

            var targets = settled
                .Where(box => string.IsNullOrEmpty(box.ExternalRef) || parcels.Contains(box.ExternalRef) == false)
                .ToList();
            if (targets.Count > 0)
            {
                _dbContext.StorageBox.Delete(world, targets);
                await _dbContext.SaveChangesAsync();
            }

            return targets.Count;
        }

        private async Task PublishWriteAsync(uint world, StorageBox box, uint host)
        {
            var response = new Response.WriteStorageBox
            {
                Box = _mapper.Map<Protocol.StorageBox>(box),
                Host = host,
                Error = (uint)ErrorCode.None
            };
            await _rabbitMqService.NotifySavedAsync(response, "storage", world);
        }

        private async Task PublishDistributedAsync(uint world, List<StorageBox> boxes, uint host)
        {
            var entries = boxes.Select(box => new Protocol.StorageWriteEntry
            {
                User = box.User,
                Box = _mapper.Map<Protocol.StorageBox>(box)
            }).ToList();

            var response = new Response.DistributeSystemStorage
            {
                Entries = entries,
                Host = host,
                Error = (uint)ErrorCode.None
            };
            await _rabbitMqService.NotifySavedAsync(response, "storage", world);
        }
    }
}
