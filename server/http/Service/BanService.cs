using Fb.Model.EnumValue;
using Http.Model;

namespace Http.Service
{
    /// <summary>
    /// Provides ban management operations for user accounts.
    /// Handles user banning and unbanning operations.
    /// </summary>
    public class BanService
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<BanService> _logger;

        /// <summary>
        /// Initializes a new instance of the <see cref="BanService"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="logger">The logger for recording operations and errors.</param>
        public BanService(DbContext dbContext, ILogger<BanService> logger)
        {
            _dbContext = dbContext;
            _logger = logger;
        }

        /// <summary>
        /// Bans a user with the specified reason and duration.
        /// If days is null, the ban is permanent.
        /// </summary>
        /// <param name="name">The character name to ban.</param>
        /// <param name="reason">The reason for the ban.</param>
        /// <param name="days">The number of days to ban the user. Null for permanent ban.</param>
        /// <returns>A result object containing the operation outcome and related data.</returns>
        public async Task<BanResult> BanUser(string name, string reason, uint? days)
        {
            // Get user ID from name
            var userId = await _dbContext.Character.GetCharacterId(name) ??
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
                CreatedDate = DateTime.Now,
                UpdatedDate = DateTime.Now
            };

            _dbContext.Ban.Set(ban);

            _logger.LogInformation("User {Name} (ID: {UserId}) has been banned. Reason: {Reason}, Expire: {ExpireDate}",
                name, userId, reason, expireDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? "Permanent");

            return new BanResult
            {
                Success = true,
                Error = ErrorCode.None,
                UserId = userId,
                ExpireDate = expireDate
            };
        }

        /// <summary>
        /// Removes a ban from a user.
        /// </summary>
        /// <param name="name">The character name to unban.</param>
        /// <returns>A result object containing the operation outcome and related data.</returns>
        public async Task<UnbanResult> UnbanUser(string name)
        {
            // Get user ID from name
            var userId = await _dbContext.Character.GetCharacterId(name) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);

            // Check if ban exists
            var ban = await _dbContext.Ban.Get(userId);
            if (ban == null || ban.Deleted)
                throw new LogicException(ErrorCode.NotFoundBan);

            // Delete ban (soft delete)
            await _dbContext.Ban.Delete(userId);

            _logger.LogInformation("User {Name} (ID: {UserId}) has been unbanned.", name, userId);

            return new UnbanResult
            {
                Success = true,
                Error = ErrorCode.None,
                UserId = userId
            };
        }
    }
}

