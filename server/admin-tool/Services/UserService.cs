using Dapper;
using Fb.Model.EnumValue;
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
        private readonly SessionService _sessionService;

        public UserService(
            DbContext dbContext,
            ILogger<UserService> logger,
            RedisService redisService,
            SessionService sessionService)
        {
            _dbContext = dbContext;
            _logger = logger;
            _redisService = redisService;
            _sessionService = sessionService;
        }

        public async Task<UserListResult> GetUsers(
            uint world,
            int page,
            int pageSize,
            string searchTerm = null)
        {
            await using var globalConn = _dbContext.GetGlobalConnection(world);

            var hasSearch = !string.IsNullOrWhiteSpace(searchTerm);
            var searchPattern = hasSearch ? $"%{searchTerm.Trim()}%" : null;

            var whereClause = hasSearch
                ? "WHERE n.`name` LIKE @searchPattern OR CAST(n.`id` AS CHAR) LIKE @searchPattern"
                : string.Empty;

            var totalCount = await globalConn.QueryFirstOrDefaultAsync<int>(
                $"""
                SELECT COUNT(*)
                FROM `name_registry` n
                {whereClause}
                """,
                new { searchPattern });

            if (totalCount == 0)
            {
                return EmptyUserListResult(page, pageSize);
            }

            var offset = (page - 1) * pageSize;
            var nameList = (await globalConn.QueryAsync<NameInfo>(
                $"""
                SELECT
                    n.`id` AS Id,
                    n.`name` AS Name
                FROM `name_registry` n
                {whereClause}
                ORDER BY n.`id` ASC
                LIMIT @pageSize OFFSET @offset
                """,
                new { searchPattern, pageSize, offset })).ToList();

            if (!nameList.Any())
            {
                return EmptyUserListResult(page, pageSize, totalCount);
            }

            return await BuildUserListResultAsync(world, page, pageSize, totalCount, nameList.Select(n => n.Id).ToList());
        }

        private async Task<UserListResult> BuildUserListResultAsync(
            uint world,
            int page,
            int pageSize,
            int totalCount,
            IReadOnlyList<uint> orderedUserIds)
        {
            var characters = await _dbContext.Character.GetMany(world, orderedUserIds);
            var banByUserId = await LoadBanInfoByUserIdsAsync(world, orderedUserIds);
            var onlineIds = await LoadOnlineUserIdsAsync(world);

            var resultUsers = new List<UserListItem>();
            foreach (var userId in orderedUserIds)
            {
                if (!characters.TryGetValue(userId, out var character))
                    continue;

                var userDetail = new UserListItem
                {
                    Id = character.Id,
                    Name = character.Name,
                    Role = (byte)character.Role,
                    Level = (ushort)character.Level,
                    Money = character.Money,
                    CreatedDate = character.CreatedDate,
                    UpdatedDate = character.UpdatedDate,
                    IsOnline = onlineIds.Contains(character.Id)
                };

                if (banByUserId.TryGetValue(userId, out var banInfo))
                {
                    userDetail.IsBanned = true;
                    userDetail.BanReason = banInfo.BanReason;
                    userDetail.BanExpireDate = banInfo.BanExpireDate;
                }

                resultUsers.Add(userDetail);
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

        private async Task<HashSet<uint>> LoadOnlineUserIdsAsync(uint world)
        {
            var onlineIds = new HashSet<uint>();
            try
            {
                var sessions = await _sessionService.GetAllSessions(world);
                foreach (var session in sessions)
                    onlineIds.Add(session.Uid);
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to load online sessions for user list in world {World}", world);
            }

            return onlineIds;
        }

        private static UserListResult EmptyUserListResult(int page, int pageSize, int totalCount = 0)
        {
            return new UserListResult
            {
                Users = new List<UserListItem>(),
                TotalCount = totalCount,
                Page = page,
                PageSize = pageSize,
                TotalPages = totalCount == 0 ? 0 : (int)Math.Ceiling(totalCount / (double)pageSize)
            };
        }

        public async Task<UserDetail> GetUserByName(uint world, string name)
        {
            var userId = await TryResolveUserIdAsync(world, name);
            if (!userId.HasValue)
                return null;

            var character = await _dbContext.Character.Get(world, userId.Value);
            if (character == null)
                return null;

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
                IsBanned = ban != null,
                BanReason = ban?.Reason,
                BanExpireDate = ban?.ExpireDate
            };
        }

        /// <summary>
        /// Resolves a character UID from an exact name or numeric UID string.
        /// </summary>
        public async Task<uint?> TryResolveUserIdAsync(uint world, string query)
        {
            if (string.IsNullOrWhiteSpace(query))
                return null;

            var term = query.Trim();

            if (uint.TryParse(term, out var uid))
            {
                var character = await _dbContext.Character.Get(world, uid);
                if (character != null)
                    return uid;
            }

            return await _dbContext.Character.GetCharacterId(world, term);
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

        public async Task<DashboardUserStats> GetDashboardStatsAsync(uint world)
        {
            var stats = new DashboardUserStats();

            try
            {
                await using var globalConn = _dbContext.GetGlobalConnection(world);
                stats.TotalAccounts = await globalConn.QuerySingleAsync<int>("SELECT COUNT(*) FROM `name_registry`");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to load dashboard account stats for world {World}", world);
            }

            try
            {
                var shardSize = _dbContext.GetShardDbSize(world);
                for (int i = 0; i < shardSize; i++)
                {
                    await using var conn = _dbContext.GetDataConnection(world, i);
                    stats.AdminCount += await conn.QuerySingleAsync<int>(
                        "SELECT COUNT(*) FROM `user` WHERE `role` >= @minRole AND `deleted` = 0",
                        new { minRole = (byte)Fb.Model.EnumValue.Role.Moderator });
                    stats.BannedCount += await conn.QuerySingleAsync<int>(
                        "SELECT COUNT(*) FROM `ban` WHERE `deleted` = 0");
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to load admin/ban count for world {World}", world);
            }

            try
            {
                var sessions = await _sessionService.GetAllSessions(world);
                stats.OnlineCount = sessions.Count;

                var onlineIds = sessions.Select(s => s.Uid).Distinct().Take(5).ToList();
                if (onlineIds.Count > 0)
                {
                    var characters = await _dbContext.Character.GetMany(world, onlineIds);
                    stats.OnlineUsers = onlineIds
                        .Where(id => characters.ContainsKey(id))
                        .Select(id =>
                        {
                            var ch = characters[id];
                            return new OnlineUserItem { Id = ch.Id, Name = ch.Name, Level = ch.Level };
                        })
                        .ToList();
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to load online users for world {World}", world);
            }

            return stats;
        }

        private async Task<Dictionary<uint, BanInfo>> LoadBanInfoByUserIdsAsync(uint world, IEnumerable<uint> userIds)
        {
            var result = new Dictionary<uint, BanInfo>();
            var ids = userIds.Distinct().ToList();
            if (ids.Count == 0)
                return result;

            var shardSize = _dbContext.GetShardDbSize(world);
            foreach (var group in ids.GroupBy(id => (int)(id % (uint)shardSize)))
            {
                await using var conn = _dbContext.GetDataConnection(world, group.Key);
                var idList = group.ToList();
                var rows = await conn.QueryAsync<BanInfo>(
                    $"""
                    SELECT
                        `user` AS UserId,
                        `reason` AS BanReason,
                        `expire_date` AS BanExpireDate
                    FROM `ban`
                    WHERE `user` IN ({string.Join(',', idList)}) AND `deleted` = 0
                    """);

                foreach (var row in rows)
                    result[row.UserId] = row;
            }

            return result;
        }
    }

    public class DashboardUserStats
    {
        public int TotalAccounts { get; set; }
        public int OnlineCount { get; set; }
        public int BannedCount { get; set; }
        public int AdminCount { get; set; }
        public List<OnlineUserItem> OnlineUsers { get; set; } = new();
    }

    public class OnlineUserItem
    {
        public uint Id { get; set; }
        public string Name { get; set; } = string.Empty;
        public ushort Level { get; set; }
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

        public bool IsOnline { get; set; }

        public string BanReason { get; set; }

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

        public string BanReason { get; set; }

        public DateTime? BanExpireDate { get; set; }
    }

    internal class NameInfo
    {
        public uint Id { get; set; }

        public string Name { get; set; } = string.Empty;
    }

    internal class BanInfo
    {
        public uint UserId { get; set; }

        public string BanReason { get; set; } = string.Empty;

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
