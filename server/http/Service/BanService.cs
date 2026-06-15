using Fb.Model.EnumValue;
using Http.Model;

namespace Http.Service
{
    public class BanService
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<BanService> _logger;
        private readonly LogService _logService;

        public BanService(DbContext dbContext, ILogger<BanService> logger, LogService logService)
        {
            _dbContext = dbContext;
            _logger = logger;
            _logService = logService;
        }


        public async Task<BanResult> Ban(uint world, string name, string reason, uint? days)
        {
            // Get user ID from name
            var userId = await _dbContext.Character.GetCharacterId(world, name) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);

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
                Deleted = false,
                CreatedDate = DateTime.Now,
                UpdatedDate = DateTime.Now
            };

            _dbContext.Ban.Set(world, ban);

            _logger.LogInformation("User {Name} (ID: {UserId}) in world {World} has been banned. Reason: {Reason}, Expire: {ExpireDate}",
                name, userId, world, reason, expireDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? "Permanent");

            // Log ban event
            await _logService.WriteAsync("ban", new
            {
                account_name = name,
                uid = userId,
                world = world,
                reason = reason,
                expire_date = expireDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? null
            });

            return new BanResult
            {
                Success = true,
                Error = ErrorCode.None,
                UserId = userId,
                ExpireDate = expireDate
            };
        }


        public async Task<UnbanResult> Unban(uint world, string name)
        {
            // Get user ID from name
            var userId = await _dbContext.Character.GetCharacterId(world, name) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);

            // Check if ban exists
            var ban = await _dbContext.Ban.Get(world, userId);
            if (ban == null || ban.Deleted)
                throw new LogicException(ErrorCode.NotFoundBan);

            // Delete ban (soft delete)
            await _dbContext.Ban.Delete(world, userId);

            _logger.LogInformation("User {Name} (ID: {UserId}) in world {World} has been unbanned.", name, userId, world);

            // Log unban event
            await _logService.WriteAsync("unban", new
            {
                account_name = name,
                uid = userId,
                world = world
            });

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
                await _dbContext.Ban.Delete(world, userId.Value);
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
