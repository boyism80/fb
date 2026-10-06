using Dapper;
using Http.Redis;
using Http.Service;
using MySqlConnector;
using StackExchange.Redis;

namespace AdminTool.Services
{
    public class GameDbResetService
    {
        private static readonly HashSet<string> PreservedTables = new(StringComparer.OrdinalIgnoreCase)
        {
            "schema_migrations"
        };

        private readonly DbContext _dbContext;
        private readonly RedisService _redisService;
        private readonly ServerStateService _serverStateService;
        private readonly CacheService _cacheService;
        private readonly ILogger<GameDbResetService> _logger;

        public GameDbResetService(
            DbContext dbContext,
            RedisService redisService,
            ServerStateService serverStateService,
            CacheService cacheService,
            ILogger<GameDbResetService> logger)
        {
            _dbContext = dbContext;
            _redisService = redisService;
            _serverStateService = serverStateService;
            _cacheService = cacheService;
            _logger = logger;
        }

        public async Task<GameDbResetResult> ResetWorldAsync(uint world)
        {
            if (world == 0)
            {
                return GameDbResetResult.Fail("유효하지 않은 월드입니다.");
            }

            if (await _serverStateService.HasRunningServers())
            {
                return GameDbResetResult.Fail("서버가 실행 중이면 DB를 초기화할 수 없습니다. 먼저 모든 서버를 종료하세요.");
            }

            var truncatedTables = 0;

            try
            {
                truncatedTables += await TruncateDatabaseAsync(_dbContext.GetGlobalConnection(world), $"world-{world}-global");

                var shardCount = _dbContext.GetShardDbSize(world);
                for (var i = 0; i < shardCount; i++)
                {
                    truncatedTables += await TruncateDatabaseAsync(
                        _dbContext.GetDataConnection(world, i),
                        $"world-{world}-data-{i}");
                }

                var deletedUnifiedRows = await DeleteUnifiedWorldRowsAsync(world);
                var clearedCacheKeys = await ClearWorldCacheAsync(world);
                await _cacheService.ClearUserSessions(world);

                _logger.LogWarning(
                    "Game DB reset completed for world {World}. TruncatedTables={TruncatedTables}, DeletedUnifiedRows={DeletedUnifiedRows}, ClearedCacheKeys={ClearedCacheKeys}",
                    world,
                    truncatedTables,
                    deletedUnifiedRows,
                    clearedCacheKeys);

                return new GameDbResetResult
                {
                    Success = true,
                    TruncatedTables = truncatedTables,
                    DeletedUnifiedRows = deletedUnifiedRows,
                    ClearedCacheKeys = clearedCacheKeys
                };
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Game DB reset failed for world {World}", world);
                return GameDbResetResult.Fail($"DB 초기화 중 오류: {ex.Message}");
            }
        }

        private async Task<int> TruncateDatabaseAsync(MySqlConnection connection, string label)
        {
            await connection.OpenAsync();
            try
            {
                var tables = (await connection.QueryAsync<string>("""
                    SELECT TABLE_NAME
                    FROM information_schema.TABLES
                    WHERE TABLE_SCHEMA = DATABASE()
                      AND TABLE_TYPE = 'BASE TABLE'
                    ORDER BY TABLE_NAME
                    """)).ToList();

                if (tables.Count == 0)
                {
                    _logger.LogInformation("No tables found for truncate target {Label}", label);
                    return 0;
                }

                await connection.ExecuteAsync("SET FOREIGN_KEY_CHECKS=0;");
                var truncated = 0;
                foreach (var table in tables)
                {
                    if (PreservedTables.Contains(table))
                        continue;

                    await connection.ExecuteAsync($"TRUNCATE TABLE `{table}`;");
                    truncated++;
                }

                await connection.ExecuteAsync("SET FOREIGN_KEY_CHECKS=1;");
                _logger.LogInformation("Truncated {Count} tables on {Label}", truncated, label);
                return truncated;
            }
            finally
            {
                await connection.CloseAsync();
            }
        }

        // The unified DB is shared by every world, so only rows of this world are deleted.
        private async Task<int> DeleteUnifiedWorldRowsAsync(uint world)
        {
            await using var connection = _dbContext.GetUnifiedConnection();
            await connection.OpenAsync();

            var deleted = 0;
            foreach (var table in new[] { "name_registry", "clan_name", "marketplace_listing", "marketplace_listing_archive", "marketplace_purchase", "marketplace_delivery" })
            {
                deleted += await connection.ExecuteAsync($"DELETE FROM `{table}` WHERE `world` = @world;", new { world });
            }

            _logger.LogInformation("Deleted {Count} unified rows for world {World}", deleted, world);
            return deleted;
        }

        private async Task<int> ClearWorldCacheAsync(uint world)
        {
            var count = 0;
            var shardSize = _redisService.GetShardSize(world);
            // Index -1 is the world global Redis (castle, system mail and system storage caches).
            for (var i = -1; i < shardSize; i++)
            {
                var redis = _redisService.GetDataConnection(world, i);
                if (redis == null)
                    continue;

                var conn = redis.Connection;
                foreach (var pattern in new[] { "fb:cache:*", Const.ReferenceCountKey, $"{Const.RedisBufferKey}:*" })
                {
                    var keys = await conn.ScanKeysAsync(pattern, 1000);
                    if (keys.Count == 0)
                        continue;

                    await conn.KeyDeleteAsync(keys.Select(x => new RedisKey(x)).ToArray());
                    count += keys.Count;
                }
            }

            return count;
        }
    }

    public class GameDbResetResult
    {
        public bool Success { get; set; }

        public string Error { get; set; }

        public int TruncatedTables { get; set; }

        public int DeletedUnifiedRows { get; set; }

        public int ClearedCacheKeys { get; set; }

        public static GameDbResetResult Fail(string error) => new()
        {
            Success = false,
            Error = error
        };
    }
}
