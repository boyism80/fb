using Fb.Model.EnumValue;
using Http.Model;
using Http.Util;

namespace Http.Service
{
    public class BanService
    {
        public const int MaxReasonLength = 256;

        private readonly DbContext _dbContext;
        private readonly ILogger<BanService> _logger;
        private readonly LogService _logService;
        private readonly DiscordNotifier _discord;
        private readonly RedisDistributedLockService _distributedLock;

        public BanService(DbContext dbContext, ILogger<BanService> logger, LogService logService, DiscordNotifier discord, RedisDistributedLockService distributedLock)
        {
            _dbContext = dbContext;
            _logger = logger;
            _logService = logService;
            _discord = discord;
            _distributedLock = distributedLock;
        }


        public async Task<BanResult> Ban(uint world, string name, string reason, uint? days, string actor, string source)
        {
            actor ??= string.Empty;
            source ??= string.Empty;

            if (TextColumn.FitsUtf8mb3(reason, MaxReasonLength) == false)
            {
                throw new ArgumentException($"Ban reason must be at most {MaxReasonLength} characters without emoji.", nameof(reason));
            }

            // Get user ID from name
            var row = await _dbContext.Character.GetCharacterRef(name) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);
            world = row.World;
            var userId = row.Id;
            DateTime? expireDate;
            await using (await _distributedLock.Lock(world, Http.Model.Ban.OperationLockKey(userId)))
            {
                var actorRole = await EnsureOutranks(actor, world, userId);
                EnsureOutranksBan(actorRole, await _dbContext.Ban.Get(world, userId));

                // Calculate expire date
                expireDate = days is > 0 ? DateTime.UtcNow.AddDays(days.Value) : null;

                // Create or update ban
                var ban = new Ban
                {
                    User = userId,
                    Reason = reason,
                    ExpireDate = expireDate,
                    ActorRole = actorRole,
                    CreatedDate = DateTime.UtcNow,
                    UpdatedDate = DateTime.UtcNow
                };

                _dbContext.Ban.Set(world, ban);
                await _dbContext.SaveChangesAsync();
            }

            _logger.LogInformation("User {Name} (ID: {UserId}) in world {World} has been banned by {Actor} ({Source}). Reason: {Reason}, Expire: {ExpireDate}",
                name, userId, world, actor, source, reason, expireDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? "Permanent");

            await _logService.WriteAsync("ban", new
            {
                action = "ban",
                source,
                actor,
                target = name,
                uid = userId,
                world,
                reason,
                expire_date = expireDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? null
            });
            var duration = expireDate.HasValue ? $"{days.Value}d" : "Permanent";
            await _discord.NotifyAsync("ban", source, actor, name, world, $"reason: {reason} / {duration}");

            return new BanResult
            {
                Success = true,
                Error = ErrorCode.None,
                UserId = userId,
                World = world,
                ExpireDate = expireDate
            };
        }


        public async Task<UnbanResult> Unban(uint world, string name, string actor, string source)
        {
            actor ??= string.Empty;
            source ??= string.Empty;

            // Get user ID from name
            var row = await _dbContext.Character.GetCharacterRef(name) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);
            world = row.World;
            var userId = row.Id;
            await using (await _distributedLock.Lock(world, Http.Model.Ban.OperationLockKey(userId)))
            {
                var actorRole = await EnsureOutranks(actor, world, userId);

                // Check if ban exists
                var ban = await _dbContext.Ban.Get(world, userId);
                if (ban == null)
                {
                    throw new LogicException(ErrorCode.NotFoundBan);
                }
                EnsureOutranksBan(actorRole, ban);

                // Delete ban (soft delete)
                _dbContext.Ban.Delete(world, userId);
                await _dbContext.SaveChangesAsync();
            }

            _logger.LogInformation("User {Name} (ID: {UserId}) in world {World} has been unbanned by {Actor} ({Source}).", name, userId, world, actor, source);

            await _logService.WriteAsync("unban", new
            {
                action = "unban",
                source,
                actor,
                target = name,
                uid = userId,
                world
            });
            await _discord.NotifyAsync("unban", source, actor, name, world, null);

            return new UnbanResult
            {
                Success = true,
                Error = ErrorCode.None,
                UserId = userId,
                World = world
            };
        }


        private async Task<Role> EnsureOutranks(string actor, uint world, uint targetId)
        {
            var target = await _dbContext.Character.Get(world, targetId) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);

            var actorRef = await _dbContext.Character.GetCharacterRef(actor);
            var actorCharacter = actorRef == null ? null : await _dbContext.Character.Get(actorRef.World, actorRef.Id);
            if (actorCharacter == null || actorCharacter.Role <= target.Role)
            {
                throw new LogicException(ErrorCode.BanNoPrivilege);
            }

            return actorCharacter.Role;
        }

        private static void EnsureOutranksBan(Role actorRole, Ban existing)
        {
            if (existing?.ActorRole == null)
            {
                return;
            }

            if (existing.ExpireDate.HasValue && existing.ExpireDate.Value <= DateTime.UtcNow)
            {
                return;
            }

            if (actorRole < existing.ActorRole.Value)
            {
                throw new LogicException(ErrorCode.BanNoPrivilege);
            }
        }


        public async Task<BanCheckResult> IsBanned(uint world, string name)
        {
            var userId = await _dbContext.Character.GetCharacterId(world, name);
            if (!userId.HasValue)
            {
                return null;
            }

            var ban = await _dbContext.Ban.Get(world, userId.Value);
            if (ban == null)
            {
                return null;
            }

            // Check if ban is expired
            if (ban.ExpireDate.HasValue && ban.ExpireDate.Value <= DateTime.UtcNow)
            {
                return null;
            }

            // User is still banned
            return new BanCheckResult
            {
                IsBanned = true,
                Reason = ban.Reason,
                ExpireDate = ban.ExpireDate
            };
        }
    }
}
