using Dapper;
using Fb.Model.EnumValue;
using Http.Extension;
using Http.Model;
using MySqlConnector;

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
        private readonly LogService _logService;

        /// <summary>
        /// Initializes a new instance of the <see cref="BanService"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="logger">The logger for recording operations and errors.</param>
        /// <param name="logService">The log service for recording ban operations (optional).</param>
        public BanService(DbContext dbContext, ILogger<BanService> logger, LogService logService = null)
        {
            _dbContext = dbContext;
            _logger = logger;
            _logService = logService;
        }


        /// <summary>
        /// Bans a user in the specified section with the specified reason and duration.
        /// If days is null, the ban is permanent.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="name">The character name to ban.</param>
        /// <param name="reason">The reason for the ban.</param>
        /// <param name="days">The number of days to ban the user. Null for permanent ban.</param>
        /// <returns>A result object containing the operation outcome and related data.</returns>
        public async Task<BanResult> Ban(string section, string name, string reason, uint? days)
        {
            // Get user ID from name
            var userId = await _dbContext.Character.GetCharacterId(section, name) ??
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

            // Use BanRepository's Set method which will use the correct connection
            // Note: This still uses the old connection method, but Ban is stored in global DB (-1)
            _dbContext.Ban.Set(section, ban);

            _logger.LogInformation("User {Name} (ID: {UserId}) in section {Section} has been banned. Reason: {Reason}, Expire: {ExpireDate}",
                name, userId, section, reason, expireDate?.ToString("yyyy-MM-dd HH:mm:ss") ?? "Permanent");

            // Log ban event
            _logService?.Write("ban", new
            {
                account_name = name,
                uid = userId,
                section = section,
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


        /// <summary>
        /// Removes a ban from a user in the specified section.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="name">The character name to unban.</param>
        /// <returns>A result object containing the operation outcome and related data.</returns>
        public async Task<UnbanResult> Unban(string section, string name)
        {
            // Get user ID from name
            var userId = await _dbContext.Character.GetCharacterId(section, name) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);

            // Check if ban exists
            var ban = await _dbContext.Ban.Get(section, userId);
            if (ban == null || ban.Deleted)
                throw new LogicException(ErrorCode.NotFoundBan);

            // Delete ban (soft delete)
            await _dbContext.Ban.Delete(section, userId);

            _logger.LogInformation("User {Name} (ID: {UserId}) in section {Section} has been unbanned.", name, userId, section);

            // Log unban event
            _logService?.Write("unban", new
            {
                account_name = name,
                uid = userId,
                section = section
            });

            return new UnbanResult
            {
                Success = true,
                Error = ErrorCode.None,
                UserId = userId
            };
        }


        /// <summary>
        /// Checks if a user is banned in the specified section and automatically removes expired bans.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="name">The character name to check.</param>
        /// <returns>A result object containing the ban status. Returns null if the user does not exist or has no ban.</returns>
        public async Task<BanCheckResult> IsBanned(string section, string name)
        {
            var userId = await _dbContext.Character.GetCharacterId(section, name);
            if (!userId.HasValue)
                return null;

            var ban = await _dbContext.Ban.Get(section, userId.Value);
            if (ban == null)
                return null;

            // Check if ban is expired
            if (ban.ExpireDate.HasValue && ban.ExpireDate.Value <= DateTime.Now)
            {
                // Ban expired, remove it
                await _dbContext.Ban.Delete(section, userId.Value);
                _logger.LogInformation("Expired ban removed for user {Name} (ID: {UserId}) in section {Section}", name, userId.Value, section);
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

