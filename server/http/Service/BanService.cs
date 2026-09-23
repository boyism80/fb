using Fb.Model.EnumValue;
using Http.Model;

namespace Http.Service
{
    public class BanService
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<BanService> _logger;
        private readonly LogService _logService;
        private readonly DiscordNotifier _discord;

        public BanService(DbContext dbContext, ILogger<BanService> logger, LogService logService, DiscordNotifier discord)
        {
            _dbContext = dbContext;
            _logger = logger;
            _logService = logService;
            _discord = discord;
        }


        public async Task<BanResult> Ban(uint world, string name, string reason, uint? days, string actor, string source)
        {
            actor ??= string.Empty;
            source ??= string.Empty;

            // Get user ID from name
            var row = await _dbContext.Character.GetCharacterRef(name) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);
            world = row.World;
            var userId = row.Id;

            // Calculate expire date
            DateTime? expireDate = null;
            if (days.HasValue)
            {
                expireDate = DateTime.Now.AddDays(days.Value);
            }

            // Create or update ban
            var ban = new Ban
            {
                User = userId,
                Reason = reason,
                ExpireDate = expireDate,
                CreatedDate = DateTime.Now,
                UpdatedDate = DateTime.Now
            };

            _dbContext.Ban.Set(world, ban);

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
            var duration = days.HasValue && days.Value > 0 ? $"{days.Value}d" : "Permanent";
            await _discord.NotifyAsync("ban", source, actor, name, world, $"reason: {reason} / {duration}");

            return new BanResult
            {
                Success = true,
                Error = ErrorCode.None,
                UserId = userId,
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

            // Check if ban exists
            var ban = await _dbContext.Ban.Get(world, userId);
            if (ban == null)
                throw new LogicException(ErrorCode.NotFoundBan);

            // Delete ban (soft delete)
            _dbContext.Ban.Delete(world, userId);

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
                UserId = userId
            };
        }


        public async Task<BanCheckResult> IsBanned(uint world, string name)
        {
            var userId = await _dbContext.Character.GetCharacterId(world, name);
            if (!userId.HasValue)
                return null;

            var ban = await _dbContext.Ban.Get(world, userId.Value);
            if (ban == null)
                return null;

            // Check if ban is expired
            if (ban.ExpireDate.HasValue && ban.ExpireDate.Value <= DateTime.Now)
            {
                _dbContext.Ban.Delete(world, userId.Value);
                await _dbContext.SaveChangesAsync();
                _logger.LogInformation("Expired ban removed for user {Name} (ID: {UserId}) in world {World}", name, userId.Value, world);
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
