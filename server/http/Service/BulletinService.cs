using Dapper;
using Http.Model;
using System.Collections.Concurrent;
using System.Data;

namespace Http.Service
{
    public class BulletinService
    {
        private readonly ConcurrentDictionary<uint, ConcurrentQueue<BulletinWriteRequest>> _writeQueues = new();
        private readonly BulletinCacheService _cacheService;
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly LogService _logService;

        public BulletinService(BulletinCacheService cacheService, IServiceScopeFactory scopeFactory, LogService logService = null)
        {
            _cacheService = cacheService;
            _scopeFactory = scopeFactory;
            _logService = logService;
        }

        public Task<bool> Write(uint world, uint section, uint user, string title, string contents)
        {
            var request = new BulletinWriteRequest
            {
                Section = section,
                World = world,
                User = user,
                Title = title,
                Contents = contents,
                CompletionSource = new TaskCompletionSource<bool>()
            };

            var queue = _writeQueues.GetOrAdd(section, _ => new ConcurrentQueue<BulletinWriteRequest>());
            queue.Enqueue(request);

            return request.CompletionSource.Task;
        }

        public async Task<int> Delete(uint world, uint bulletinSection, uint id, uint user, bool ignoreOwner = false)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            try
            {
                await using var conn = dbContext.Connection(world, null);
                await conn.OpenAsync();

                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("id", id);
                dynamicParams.Add("user", user);
                dynamicParams.Add("ignore_owner", ignoreOwner ? 1 : 0);

                var result = await conn.QueryFirstOrDefaultAsync<int>(
                    "USP_BULLETIN_DELETE",
                    dynamicParams,
                    commandType: CommandType.StoredProcedure);

                // Delete from Redis cache if successful
                if (result == 1)
                {
                    await _cacheService.DeleteArticlesBatchAsync(world, new List<(uint bulletinSection, uint id)> { (bulletinSection, id) });

                    // Log bulletin delete event
                    _logService?.Write("bulletin_delete", new
                    {
                        world = world,
                        bulletinSection = bulletinSection,
                        article_id = id,
                        user_id = user,
                        ignore_owner = ignoreOwner
                    });
                }

                return result;
            }
            catch (Exception)
            {
                // Log error and return failure code
                return -4;
            }
        }

        public async Task<int> DeleteBatch(uint world, uint bulletinSection, List<uint> ids, uint user, bool ignoreOwner = false)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            try
            {
                await using var conn = dbContext.Connection(world, null);
                await conn.OpenAsync();

                var deletedIds = new List<(uint bulletinSection, uint id)>();
                var successCount = 0;

                foreach (var id in ids)
                {
                    var dynamicParams = new DynamicParameters();
                    dynamicParams.Add("id", id);
                    dynamicParams.Add("user", user);
                    dynamicParams.Add("ignore_owner", ignoreOwner ? 1 : 0);

                    var result = await conn.QueryFirstOrDefaultAsync<int>(
                        "USP_BULLETIN_DELETE",
                        dynamicParams,
                        commandType: CommandType.StoredProcedure);

                    if (result == 1)
                    {
                        deletedIds.Add((bulletinSection, id));
                        successCount++;
                    }
                }

                // Delete from Redis cache for all successfully deleted articles
                if (deletedIds.Any())
                {
                    await _cacheService.DeleteArticlesBatchAsync(world, deletedIds);

                    // Log bulletin batch delete event
                    _logService?.Write("bulletin_delete_batch", new
                    {
                        world = world,
                        bulletinSection = bulletinSection,
                        article_ids = deletedIds.Select(x => x.id).ToList(),
                        user_id = user,
                        ignore_owner = ignoreOwner,
                        count = successCount
                    });
                }

                return successCount;
            }
            catch (Exception)
            {
                return -4;
            }
        }

        public async Task<int> Update(uint world, uint bulletinSection, uint id, uint user, string title, string contents, bool ignoreOwner = false)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            try
            {
                await using var conn = dbContext.Connection(world, null);
                await conn.OpenAsync();

                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("p_id", id);
                dynamicParams.Add("p_user", user);
                dynamicParams.Add("p_title", title);
                dynamicParams.Add("p_contents", contents);
                dynamicParams.Add("p_ignore_owner", ignoreOwner ? 1 : 0);

                var result = await conn.QueryFirstOrDefaultAsync<int>(
                    "USP_BULLETIN_UPDATE",
                    dynamicParams,
                    commandType: CommandType.StoredProcedure);

                if (result == 1)
                {
                    var (article, _) = await LoadArticleFromDatabase(world, bulletinSection, id);
                    if (article != null)
                    {
                        await _cacheService.SetArticleAsync(world, bulletinSection, id, article);
                    }
                    else
                    {
                        await _cacheService.DeleteArticlesBatchAsync(world, new List<(uint bulletinSection, uint id)> { (bulletinSection, id) });
                    }

                    // Log bulletin update event
                    _logService?.Write("bulletin_update", new
                    {
                        world = world,
                        bulletinSection = bulletinSection,
                        article_id = id,
                        user_id = user,
                        ignore_owner = ignoreOwner
                    });
                }

                return result;
            }
            catch (Exception)
            {
                return -4;
            }
        }

        public Dictionary<uint, List<BulletinWriteRequest>> DequeueBatch(int maxBatchSize)
        {
            var writes = new Dictionary<uint, List<BulletinWriteRequest>>();
            foreach (var (bulletinSection, queue) in _writeQueues)
            {
                var batch = new List<BulletinWriteRequest>();
                while (batch.Count < maxBatchSize && queue.TryDequeue(out var request))
                {
                    batch.Add(request);
                }
                if (batch.Count > 0)
                    writes[bulletinSection] = batch;
            }

            return writes;
        }

        public async Task<List<Bulletin>> GetArticleListAsync(uint world, uint bulletinSection, ushort offset, string searchQuery = null)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.Connection(world, null);

            List<Bulletin> articleList;

            if (string.IsNullOrWhiteSpace(searchQuery))
            {
                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("bulletinSection", bulletinSection);
                dynamicParams.Add("position", offset);

                var articles = await conn.QueryAsync<Bulletin>(
                    "USP_BULLETIN_GET_LIST",
                    dynamicParams,
                    commandType: CommandType.StoredProcedure);

                articleList = articles.ToList();
            }
            else
            {
                // Search by title, contents, or author name
                // Note: character table JOIN is not allowed as they may be in different databases
                // For user name search, we first look up the user ID, then search by user ID
                var searchPattern = $"%{searchQuery}%";
                uint? userId = null;

                // Try to find user ID by name if search query might be a user name
                var foundUserId = await dbContext.Character.GetCharacterId(world, searchQuery);
                if (foundUserId.HasValue)
                {
                    userId = foundUserId.Value;
                }

                var sql = @"
                    SELECT b.id, b.bulletinSection, b.user, b.title, b.contents, b.created_date, b.updated_date, b.deleted
                    FROM bulletin b
                    WHERE b.bulletinSection = @bulletinSection 
                      AND b.deleted = 0
                      AND @position >= b.id
                      AND (b.title LIKE @search OR b.contents LIKE @search";

                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("bulletinSection", bulletinSection);
                dynamicParams.Add("search", searchPattern);
                dynamicParams.Add("position", offset);

                if (userId.HasValue)
                {
                    sql += " OR b.user = @userId";
                    dynamicParams.Add("userId", userId.Value);
                }

                sql += @")
                    ORDER BY b.id DESC
                    LIMIT 20";

                var articles = await conn.QueryAsync<Bulletin>(sql, dynamicParams);
                articleList = articles.ToList();
            }

            if (articleList.Any())
            {
                var userIds = articleList.Select(a => a.User).Distinct().ToList();
                var userNames = await dbContext.Character.GetName(world, userIds);

                foreach (var article in articleList)
                {
                    article.UserName = userNames.TryGetValue(article.User, out var name) ? name : string.Empty;
                }
            }

            return articleList;
        }

        public async Task<(Bulletin Article, bool Next)> GetArticleAsync(uint world, uint bulletinSection, uint id)
        {
            bool? nextFlagFromLoader = null;

            var article = await _cacheService.GetArticleAsync(
                world,
                bulletinSection,
                id,
                async () =>
                {
                    var (dbArticle, next) = await LoadArticleFromDatabase(world, bulletinSection, id);
                    nextFlagFromLoader = next;
                    return dbArticle;
                });

            if (article == null)
                return (null, false);

            var nextFlag = nextFlagFromLoader ?? await LoadNextFlagAsync(world, bulletinSection, id);
            return (article, nextFlag);
        }

        private async Task<(Bulletin Article, bool Next)> LoadArticleFromDatabase(uint world, uint bulletinSection, uint id)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.Connection(world, null);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("bulletinSection", bulletinSection);
            dynamicParams.Add("article", id);
            await using var reader = await conn.QueryMultipleAsync(
                "USP_BULLETIN_GET",
                dynamicParams,
                commandType: CommandType.StoredProcedure);

            var dbArticle = await reader.ReadFirstOrDefaultAsync<Bulletin>();
            if (dbArticle == null)
                return (null, false);

            dbArticle.UserName = await dbContext.Character.GetName(world, dbArticle.User) ?? string.Empty;
            var next = await reader.ReadFirstAsync<bool>();

            return (dbArticle, next);
        }

        private async Task<bool> LoadNextFlagAsync(uint world, uint bulletinSection, uint id)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.Connection(world, null);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("bulletinSection", bulletinSection);
            dynamicParams.Add("article", id);
            await using var reader = await conn.QueryMultipleAsync(
                "USP_BULLETIN_GET",
                dynamicParams,
                commandType: CommandType.StoredProcedure);

            await reader.ReadFirstOrDefaultAsync<Bulletin>();
            return await reader.ReadFirstAsync<bool>();
        }
    }
}

