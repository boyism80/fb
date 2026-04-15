using Dapper;
using Http.Redis.Key;
using Http.Service;
using StackExchange.Redis;

namespace AdminTool.Services
{
    public class UserService
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<UserService> _logger;
        private readonly RedisService _redisService;

        public UserService(DbContext dbContext, ILogger<UserService> logger, RedisService redisService)
        {
            _dbContext = dbContext;
            _logger = logger;
            _redisService = redisService;
        }

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

            // Get total count from name_registry table (global DB)
            var countQuery = $"""
                SELECT COUNT(*) 
                FROM `name_registry` n
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
                FROM `name_registry` n
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

    public class UserListItem
    {
        public uint Id { get; set; }

        public string Name { get; set; } = string.Empty;

        public byte Role { get; set; }

        public ushort Level { get; set; }

        public ulong Money { get; set; }

        public DateTime CreatedDate { get; set; }

        public DateTime UpdatedDate { get; set; }

        public bool IsBanned { get; set; }

        public string? BanReason { get; set; }

        public DateTime? BanExpireDate { get; set; }
    }

    public class UserDetail
    {
        public uint Id { get; set; }

        public string Name { get; set; } = string.Empty;

        public Fb.Model.EnumValue.Role Role { get; set; }

        public ushort Level { get; set; }

        public ulong Money { get; set; }

        public DateTime CreatedDate { get; set; }

        public DateTime UpdatedDate { get; set; }

        public bool IsBanned { get; set; }

        public string? BanReason { get; set; }

        public DateTime? BanExpireDate { get; set; }
    }

    internal class NameWithBanInfo
    {
        public uint Id { get; set; }

        public string Name { get; set; } = string.Empty;

        public int IsBanned { get; set; }

        public string? BanReason { get; set; }

        public DateTime? BanExpireDate { get; set; }
    }

    public class UserListResult
    {
        public List<UserListItem> Users { get; set; } = new();

        public int TotalCount { get; set; }

        public int Page { get; set; }

        public int PageSize { get; set; }

        public int TotalPages { get; set; }
    }
}
