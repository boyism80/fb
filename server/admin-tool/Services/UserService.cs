using Dapper;
using Http;
using Http.Model;
using Http.Service;
using MySqlConnector;

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

        /// <summary>
        /// Initializes a new instance of the <see cref="UserService"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="logger">The logger for recording operations.</param>
        public UserService(DbContext dbContext, ILogger<UserService> logger)
        {
            _dbContext = dbContext;
            _logger = logger;
        }

        /// <summary>
        /// Retrieves a paginated list of users with optional search filtering.
        /// </summary>
        /// <param name="page">The page number (1-based).</param>
        /// <param name="pageSize">The number of items per page.</param>
        /// <param name="searchTerm">Optional search term to filter by character name.</param>
        /// <returns>A result object containing the user list and pagination information.</returns>
        public async Task<UserListResult> GetUsers(int page, int pageSize, string? searchTerm = null)
        {
            await using var conn = _dbContext.Connection(-1);

            var offset = (page - 1) * pageSize;
            var whereClause = "WHERE `deleted` = 0";
            var searchParam = "";

            if (!string.IsNullOrWhiteSpace(searchTerm))
            {
                whereClause += " AND `name` LIKE @searchTerm";
                searchParam = $"%{searchTerm}%";
            }

            // Get total count
            var countQuery = $"SELECT COUNT(*) FROM `user` {whereClause}";
            var totalCount = await conn.QueryFirstOrDefaultAsync<int>(countQuery, new { searchTerm = searchParam });

            // Get paginated results
            var query = $"""
                SELECT 
                    `id`,
                    `name`,
                    `role`,
                    `level`,
                    `money`,
                    `created_date`,
                    `updated_date`
                FROM `user`
                {whereClause}
                ORDER BY `id` DESC
                LIMIT @pageSize OFFSET @offset
                """;

            var users = await conn.QueryAsync<UserListItem>(query, new
            {
                searchTerm = searchParam,
                pageSize,
                offset
            });

            return new UserListResult
            {
                Users = users.ToList(),
                TotalCount = totalCount,
                Page = page,
                PageSize = pageSize,
                TotalPages = (int)Math.Ceiling(totalCount / (double)pageSize)
            };
        }

        /// <summary>
        /// Retrieves a user by their character name.
        /// </summary>
        /// <param name="name">The character name to look up.</param>
        /// <returns>The user if found; otherwise, null.</returns>
        public async Task<UserDetail?> GetUserByName(string name)
        {
            var userId = await _dbContext.Character.GetCharacterId(name);
            if (!userId.HasValue)
                return null;

            var character = await _dbContext.Character.Get(userId.Value);
            if (character == null)
                return null;

            // Get ban information if exists
            var ban = await _dbContext.Ban.Get(userId.Value);

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

