using Dapper;
using Http.Redis.Key;
using Http.Service;
using StackExchange.Redis;

namespace AdminTool.Services
{
    /// <summary>
    /// Provides user management services for the admin tool.
    /// Handles user listing, searching, and retrieval operations.
    /// </summary>
    public class UserService
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<UserService> _logger;
        private readonly RedisService _redisService;

        /// <summary>
        /// Initializes a new instance of the <see cref="UserService"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="logger">The logger for recording operations.</param>
        /// <param name="redisService">The Redis service for session lookups.</param>
        public UserService(DbContext dbContext, ILogger<UserService> logger, RedisService redisService)
        {
            _dbContext = dbContext;
            _logger = logger;
            _redisService = redisService;
        }

        /// <summary>
        /// Retrieves a paginated list of users with optional search filtering.
        /// Uses the global 'name' table to efficiently query across sharded 'user' tables.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified.</param>
        /// <param name="page">The page number (1-based).</param>
        /// <param name="pageSize">The number of items per page.</param>
        /// <param name="searchTerm">Optional search term to filter by character name.</param>
        /// <returns>A result object containing the user list and pagination information.</returns>
        public async Task<UserListResult> GetUsers(uint world, int page, int pageSize, string? searchTerm = null)
        {
            await using var globalConn = _dbContext.GetGlobalConnection(world);

            var offset = (page - 1) * pageSize;
            var whereClause = "";
            var searchParam = "";

            if (!string.IsNullOrWhiteSpace(searchTerm))
            {
                whereClause = "WHERE n.`name` LIKE @searchTerm";
                searchParam = $"%{searchTerm}%";
            }

            // Get total count from name table (global DB)
            var countQuery = $"""
                SELECT COUNT(*) 
                FROM `name` n
                {whereClause}
                """;
            var totalCount = await globalConn.QueryFirstOrDefaultAsync<int>(countQuery, new { searchTerm = searchParam });

            // Get paginated name IDs with ban information from global DB
            var nameQuery = $"""
                SELECT 
                    n.`id` AS Id,
                    n.`name` AS Name,
                    CASE WHEN b.`user` IS NOT NULL AND b.`deleted` = 0 THEN 1 ELSE 0 END AS IsBanned,
                    b.`reason` AS BanReason,
                    b.`expire_date` AS BanExpireDate
                FROM `name` n
                LEFT JOIN `ban` b ON n.`id` = b.`user` AND b.`deleted` = 0
                {whereClause}
                ORDER BY n.`id` DESC
                LIMIT @pageSize OFFSET @offset
                """;

            var nameResults = await globalConn.QueryAsync<NameWithBanInfo>(nameQuery, new
            {
                searchTerm = searchParam,
                pageSize,
                offset
            });

            var nameList = nameResults.ToList();
            if (!nameList.Any())
            {
                return new UserListResult
                {
                    Users = new List<UserListItem>(),
                    TotalCount = totalCount,
                    Page = page,
                    PageSize = pageSize,
                    TotalPages = (int)Math.Ceiling(totalCount / (double)pageSize)
                };
            }

            // Get user details via Character repository (Redis cache) so list matches role changes.
            var userIds = nameList.Select(n => n.Id).ToList();
            var userDetailsDict = new Dictionary<uint, UserListItem>();

            foreach (var userId in userIds)
            {
                var character = await _dbContext.Character.Get(world, userId);
                if (character == null || character.Deleted)
                    continue;

                userDetailsDict[userId] = new UserListItem
                {
                    Id = character.Id,
                    Name = character.Name,
                    Role = (byte)character.Role,
                    Level = (ushort)character.Level,
                    Money = character.Money,
                    CreatedDate = character.CreatedDate,
                    UpdatedDate = character.UpdatedDate
                };
            }

            // Combine name/ban info with user details
            var resultUsers = new List<UserListItem>();
            foreach (var nameInfo in nameList)
            {
                if (userDetailsDict.TryGetValue(nameInfo.Id, out var userDetail))
                {
                    userDetail.IsBanned = nameInfo.IsBanned == 1;
                    userDetail.BanReason = nameInfo.BanReason;
                    userDetail.BanExpireDate = nameInfo.BanExpireDate;
                    resultUsers.Add(userDetail);
                }
            }

            return new UserListResult
            {
                Users = resultUsers,
                TotalCount = totalCount,
                Page = page,
                PageSize = pageSize,
                TotalPages = (int)Math.Ceiling(totalCount / (double)pageSize)
            };
        }

        /// <summary>
        /// Retrieves a user by their character name.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified.</param>
        /// <param name="name">The character name to look up.</param>
        /// <returns>The user if found; otherwise, null.</returns>
        public async Task<UserDetail?> GetUserByName(uint world, string name)
        {
            var userId = await _dbContext.Character.GetCharacterId(world, name);
            if (!userId.HasValue)
                return null;

            var character = await _dbContext.Character.Get(world, userId.Value);
            if (character == null)
                return null;

            // Get ban information if exists
            var ban = await _dbContext.Ban.Get(world, userId.Value);

            return new UserDetail
            {
                Id = character.Id,
                Name = character.Name,
                Role = character.Role,
                Level = character.Level,
                Money = character.Money,
                CreatedDate = character.CreatedDate,
                UpdatedDate = character.UpdatedDate,
                IsBanned = ban != null && !ban.Deleted,
                BanReason = ban?.Reason,
                BanExpireDate = ban?.ExpireDate
            };
        }

        /// <summary>
        /// Determines whether a user is currently online by inspecting the session cache.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified.</param>
        /// <param name="userName">The character name to inspect.</param>
        /// <returns>True if the user has an active session; otherwise, false.</returns>
        public async Task<bool> IsOnline(uint world, string userName)
        {
            if (string.IsNullOrWhiteSpace(userName))
                return false;

            try
            {
                var redis = _redisService.GetGlobalConnection(world);
                if (redis == null)
                {
                    _logger.LogWarning("Redis service instance is not available for IsOnline check.");
                    return false;
                }

                var key = new SessionKey().Key;
                var connection = redis.Connection;
                return await connection.HashExistsAsync(new RedisKey(key), new RedisValue(userName));
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to check online status for user {UserName}", userName);
                return false;
            }
        }
    }

    /// <summary>
    /// Represents a user list item for display purposes.
    /// </summary>
    public class UserListItem
    {
        /// <summary>
        /// Gets or sets the user ID.
        /// </summary>
        public uint Id { get; set; }

        /// <summary>
        /// Gets or sets the character name.
        /// </summary>
        public string Name { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the user role.
        /// </summary>
        public byte Role { get; set; }

        /// <summary>
        /// Gets or sets the character level.
        /// </summary>
        public ushort Level { get; set; }

        /// <summary>
        /// Gets or sets the character's money.
        /// </summary>
        public ulong Money { get; set; }

        /// <summary>
        /// Gets or sets the character creation date.
        /// </summary>
        public DateTime CreatedDate { get; set; }

        /// <summary>
        /// Gets or sets the last update date.
        /// </summary>
        public DateTime UpdatedDate { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the user is currently banned.
        /// </summary>
        public bool IsBanned { get; set; }

        /// <summary>
        /// Gets or sets the ban reason if the user is banned.
        /// </summary>
        public string? BanReason { get; set; }

        /// <summary>
        /// Gets or sets the ban expiration date if the user is banned.
        /// </summary>
        public DateTime? BanExpireDate { get; set; }
    }

    /// <summary>
    /// Represents detailed user information.
    /// </summary>
    public class UserDetail
    {
        /// <summary>
        /// Gets or sets the user ID.
        /// </summary>
        public uint Id { get; set; }

        /// <summary>
        /// Gets or sets the character name.
        /// </summary>
        public string Name { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the user role.
        /// </summary>
        public Fb.Model.EnumValue.Role Role { get; set; }

        /// <summary>
        /// Gets or sets the character level.
        /// </summary>
        public ushort Level { get; set; }

        /// <summary>
        /// Gets or sets the character's money.
        /// </summary>
        public ulong Money { get; set; }

        /// <summary>
        /// Gets or sets the character creation date.
        /// </summary>
        public DateTime CreatedDate { get; set; }

        /// <summary>
        /// Gets or sets the last update date.
        /// </summary>
        public DateTime UpdatedDate { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the user is currently banned.
        /// </summary>
        public bool IsBanned { get; set; }

        /// <summary>
        /// Gets or sets the ban reason if the user is banned.
        /// </summary>
        public string? BanReason { get; set; }

        /// <summary>
        /// Gets or sets the ban expiration date if the user is banned.
        /// </summary>
        public DateTime? BanExpireDate { get; set; }
    }

    /// <summary>
    /// Represents name table entry with ban information.
    /// </summary>
    internal class NameWithBanInfo
    {
        /// <summary>
        /// Gets or sets the name ID (user ID).
        /// </summary>
        public uint Id { get; set; }

        /// <summary>
        /// Gets or sets the character name.
        /// </summary>
        public string Name { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets whether the user is banned (1 = banned, 0 = not banned).
        /// </summary>
        public int IsBanned { get; set; }

        /// <summary>
        /// Gets or sets the ban reason if the user is banned.
        /// </summary>
        public string? BanReason { get; set; }

        /// <summary>
        /// Gets or sets the ban expiration date if the user is banned.
        /// </summary>
        public DateTime? BanExpireDate { get; set; }
    }

    /// <summary>
    /// Represents the result of a user list query with pagination information.
    /// </summary>
    public class UserListResult
    {
        /// <summary>
        /// Gets or sets the list of users.
        /// </summary>
        public List<UserListItem> Users { get; set; } = new();

        /// <summary>
        /// Gets or sets the total number of users matching the query.
        /// </summary>
        public int TotalCount { get; set; }

        /// <summary>
        /// Gets or sets the current page number.
        /// </summary>
        public int Page { get; set; }

        /// <summary>
        /// Gets or sets the page size.
        /// </summary>
        public int PageSize { get; set; }

        /// <summary>
        /// Gets or sets the total number of pages.
        /// </summary>
        public int TotalPages { get; set; }
    }
}

