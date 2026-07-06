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
            string searchTerm = null,
            string sortBy = null,
            bool sortDescending = true,
            string statusFilter = "all",
            string roleFilter = "all")
        {
            var column = NormalizeSortColumn(sortBy);
            if (HasUserListFilters(statusFilter, roleFilter))
            {
                return await GetUsersFilteredAsync(
                    world,
                    page,
                    pageSize,
                    searchTerm,
                    column,
                    sortDescending,
                    statusFilter,
                    roleFilter);
            }

            if (column is "id" or "name")
            {
                return await GetUsersSortedByNameRegistryAsync(
                    world, page, pageSize, searchTerm, column, sortDescending);
            }

            return await GetUsersSortedByCharacterFieldAsync(
                world, page, pageSize, searchTerm, column, sortDescending);
        }

        private static bool HasUserListFilters(string statusFilter, string roleFilter)
        {
            return !string.Equals(statusFilter, "all", StringComparison.OrdinalIgnoreCase)
                || !string.Equals(roleFilter, "all", StringComparison.OrdinalIgnoreCase);
        }

        private async Task<UserListResult> GetUsersFilteredAsync(
            uint world,
            int page,
            int pageSize,
            string searchTerm,
            string sortColumn,
            bool sortDescending,
            string statusFilter,
            string roleFilter)
        {
            HashSet<uint> searchIds = null;
            if (!string.IsNullOrWhiteSpace(searchTerm))
            {
                await using var globalConn = _dbContext.GetGlobalConnection(world);
                var ids = await globalConn.QueryAsync<uint>(
                    """
                    SELECT `id`
                    FROM `name_registry`
                    WHERE `name` LIKE @searchTerm
                    """,
                    new { searchTerm = $"%{searchTerm}%" });
                searchIds = ids.ToHashSet();
                if (searchIds.Count == 0)
                {
                    return EmptyUserListResult(page, pageSize);
                }
            }

            var sortRows = await LoadUserSortRowsAsync(world, searchIds);
            if (sortRows.Count == 0)
            {
                return EmptyUserListResult(page, pageSize);
            }

            var onlineIds = await LoadOnlineUserIdsAsync(world);
            var bannedIds = await LoadAllBannedUserIdsAsync(world);
            sortRows = ApplyUserListFilters(sortRows, statusFilter, roleFilter, onlineIds, bannedIds);
            if (sortRows.Count == 0)
            {
                return EmptyUserListResult(page, pageSize);
            }

            Dictionary<uint, string> nameById = null;
            if (sortColumn == "name")
            {
                nameById = await LoadNamesByIdsAsync(world, sortRows.Select(r => r.Id).ToList());
            }

            var sortedIds = SortUserSortRows(sortRows, sortColumn, sortDescending, onlineIds, bannedIds, nameById)
                .Select(r => r.Id)
                .ToList();

            var totalCount = sortedIds.Count;
            var offset = (page - 1) * pageSize;
            var pageIds = sortedIds.Skip(offset).Take(pageSize).ToList();
            if (!pageIds.Any())
            {
                return EmptyUserListResult(page, pageSize, totalCount);
            }

            return await BuildUserListResultAsync(world, page, pageSize, totalCount, pageIds);
        }

        private async Task<Dictionary<uint, string>> LoadNamesByIdsAsync(uint world, IReadOnlyList<uint> userIds)
        {
            if (userIds == null || userIds.Count == 0)
            {
                return new Dictionary<uint, string>();
            }

            await using var globalConn = _dbContext.GetGlobalConnection(world);
            var rows = await globalConn.QueryAsync<NameInfo>(
                $"""
                SELECT
                    `id` AS Id,
                    `name` AS Name
                FROM `name_registry`
                WHERE `id` IN ({string.Join(',', userIds)})
                """);

            return rows.ToDictionary(row => row.Id, row => row.Name);
        }

        private static List<UserSortRow> ApplyUserListFilters(
            List<UserSortRow> rows,
            string statusFilter,
            string roleFilter,
            HashSet<uint> onlineIds,
            HashSet<uint> bannedIds)
        {
            IEnumerable<UserSortRow> query = rows;

            if (string.Equals(statusFilter, "online", StringComparison.OrdinalIgnoreCase))
            {
                query = query.Where(row => !bannedIds.Contains(row.Id) && onlineIds.Contains(row.Id));
            }
            else if (string.Equals(statusFilter, "offline", StringComparison.OrdinalIgnoreCase))
            {
                query = query.Where(row => !bannedIds.Contains(row.Id) && !onlineIds.Contains(row.Id));
            }
            else if (string.Equals(statusFilter, "banned", StringComparison.OrdinalIgnoreCase))
            {
                query = query.Where(row => bannedIds.Contains(row.Id));
            }

            if (string.Equals(roleFilter, "user", StringComparison.OrdinalIgnoreCase))
            {
                query = query.Where(row => row.Role == (byte)Fb.Model.EnumValue.Role.User);
            }
            else if (string.Equals(roleFilter, "staff", StringComparison.OrdinalIgnoreCase))
            {
                query = query.Where(row => row.Role >= (byte)Fb.Model.EnumValue.Role.Moderator);
            }

            return query.ToList();
        }

        private async Task<UserListResult> GetUsersSortedByNameRegistryAsync(
            uint world,
            int page,
            int pageSize,
            string searchTerm,
            string sortColumn,
            bool sortDescending)
        {
            await using var globalConn = _dbContext.GetGlobalConnection(world);

            var whereClause = "";
            var searchParam = "";
            if (!string.IsNullOrWhiteSpace(searchTerm))
            {
                whereClause = "WHERE n.`name` LIKE @searchTerm";
                searchParam = $"%{searchTerm}%";
            }

            var totalCount = await globalConn.QueryFirstOrDefaultAsync<int>(
                $"""
                SELECT COUNT(*)
                FROM `name_registry` n
                {whereClause}
                """,
                new { searchTerm = searchParam });

            if (totalCount == 0)
            {
                return EmptyUserListResult(page, pageSize);
            }

            var orderBy = sortColumn == "name"
                ? sortDescending ? "n.`name` DESC" : "n.`name` ASC"
                : sortDescending ? "n.`id` DESC" : "n.`id` ASC";

            var offset = (page - 1) * pageSize;
            var nameList = (await globalConn.QueryAsync<NameInfo>(
                $"""
                SELECT
                    n.`id` AS Id,
                    n.`name` AS Name
                FROM `name_registry` n
                {whereClause}
                ORDER BY {orderBy}
                LIMIT @pageSize OFFSET @offset
                """,
                new { searchTerm = searchParam, pageSize, offset })).ToList();

            if (!nameList.Any())
            {
                return EmptyUserListResult(page, pageSize, totalCount);
            }

            return await BuildUserListResultAsync(world, page, pageSize, totalCount, nameList.Select(n => n.Id).ToList());
        }

        private async Task<UserListResult> GetUsersSortedByCharacterFieldAsync(
            uint world,
            int page,
            int pageSize,
            string searchTerm,
            string sortColumn,
            bool sortDescending)
        {
            HashSet<uint> searchIds = null;
            if (!string.IsNullOrWhiteSpace(searchTerm))
            {
                await using var globalConn = _dbContext.GetGlobalConnection(world);
                var ids = await globalConn.QueryAsync<uint>(
                    """
                    SELECT `id`
                    FROM `name_registry`
                    WHERE `name` LIKE @searchTerm
                    """,
                    new { searchTerm = $"%{searchTerm}%" });
                searchIds = ids.ToHashSet();
                if (searchIds.Count == 0)
                {
                    return EmptyUserListResult(page, pageSize);
                }
            }

            var sortRows = await LoadUserSortRowsAsync(world, searchIds);
            if (sortRows.Count == 0)
            {
                return EmptyUserListResult(page, pageSize);
            }

            var onlineIds = await LoadOnlineUserIdsAsync(world);
            HashSet<uint> bannedIds = null;
            if (sortColumn == "status")
            {
                bannedIds = await LoadAllBannedUserIdsAsync(world);
            }

            var sortedIds = SortUserSortRows(sortRows, sortColumn, sortDescending, onlineIds, bannedIds)
                .Select(r => r.Id)
                .ToList();

            var totalCount = sortedIds.Count;
            var offset = (page - 1) * pageSize;
            var pageIds = sortedIds.Skip(offset).Take(pageSize).ToList();
            if (!pageIds.Any())
            {
                return EmptyUserListResult(page, pageSize, totalCount);
            }

            return await BuildUserListResultAsync(world, page, pageSize, totalCount, pageIds);
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

        private async Task<List<UserSortRow>> LoadUserSortRowsAsync(uint world, HashSet<uint> filterIds)
        {
            var shardSize = _dbContext.GetShardDbSize(world);
            var rows = new List<UserSortRow>();

            if (filterIds != null)
            {
                foreach (var group in filterIds.GroupBy(id => (int)(id % (uint)shardSize)))
                {
                    await using var conn = _dbContext.GetDataConnection(world, group.Key);
                    var idList = group.ToList();
                    var shardRows = await conn.QueryAsync<UserSortRow>(
                        $"""
                        SELECT
                            `id` AS Id,
                            `level` AS Level,
                            `role` AS Role,
                            `updated_date` AS UpdatedDate
                        FROM `user`
                        WHERE `deleted` = 0 AND `id` IN ({string.Join(',', idList)})
                        """);
                    rows.AddRange(shardRows);
                }

                return rows;
            }

            for (int i = 0; i < shardSize; i++)
            {
                await using var conn = _dbContext.GetDataConnection(world, i);
                var shardRows = await conn.QueryAsync<UserSortRow>(
                    """
                    SELECT
                        `id` AS Id,
                        `level` AS Level,
                        `role` AS Role,
                        `updated_date` AS UpdatedDate
                    FROM `user`
                    WHERE `deleted` = 0
                    """);
                rows.AddRange(shardRows);
            }

            return rows;
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

        private async Task<HashSet<uint>> LoadAllBannedUserIdsAsync(uint world)
        {
            var bannedIds = new HashSet<uint>();
            var shardSize = _dbContext.GetShardDbSize(world);
            for (int i = 0; i < shardSize; i++)
            {
                await using var conn = _dbContext.GetDataConnection(world, i);
                var ids = await conn.QueryAsync<uint>(
                    "SELECT `user` FROM `ban` WHERE `deleted` = 0");
                foreach (var id in ids)
                    bannedIds.Add(id);
            }

            return bannedIds;
        }

        private static List<UserSortRow> SortUserSortRows(
            List<UserSortRow> rows,
            string sortColumn,
            bool sortDescending,
            HashSet<uint> onlineIds,
            HashSet<uint> bannedIds,
            Dictionary<uint, string> nameById = null)
        {
            IEnumerable<UserSortRow> ordered = sortColumn switch
            {
                "name" => sortDescending
                    ? rows.OrderByDescending(r => nameById?.GetValueOrDefault(r.Id) ?? string.Empty)
                    : rows.OrderBy(r => nameById?.GetValueOrDefault(r.Id) ?? string.Empty),
                "level" => sortDescending
                    ? rows.OrderByDescending(r => r.Level)
                    : rows.OrderBy(r => r.Level),
                "role" => sortDescending
                    ? rows.OrderByDescending(r => r.Role)
                    : rows.OrderBy(r => r.Role),
                "updatedDate" => sortDescending
                    ? rows.OrderByDescending(r => r.UpdatedDate)
                    : rows.OrderBy(r => r.UpdatedDate),
                "status" => sortDescending
                    ? rows.OrderByDescending(r => GetStatusSortRank(r.Id, onlineIds, bannedIds ?? new HashSet<uint>()))
                    : rows.OrderBy(r => GetStatusSortRank(r.Id, onlineIds, bannedIds ?? new HashSet<uint>())),
                _ => sortDescending
                    ? rows.OrderByDescending(r => r.Id)
                    : rows.OrderBy(r => r.Id),
            };

            return ordered.ToList();
        }

        private static int GetStatusSortRank(uint userId, HashSet<uint> onlineIds, HashSet<uint> bannedIds)
        {
            if (bannedIds.Contains(userId))
                return 0;

            if (onlineIds.Contains(userId))
                return 2;

            return 1;
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

        public static string NormalizeSortColumn(string sortBy)
        {
            return sortBy?.Trim().ToLowerInvariant() switch
            {
                "name" => "name",
                "level" => "level",
                "role" => "role",
                "status" => "status",
                "updateddate" or "updated" => "updatedDate",
                "id" or "uid" => "id",
                _ => "id",
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

    internal class UserSortRow
    {
        public uint Id { get; set; }

        public ushort Level { get; set; }

        public byte Role { get; set; }

        public DateTime UpdatedDate { get; set; }
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
