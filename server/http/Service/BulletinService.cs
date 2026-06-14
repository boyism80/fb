using Dapper;
using Http.Model;
using System.Collections.Concurrent;
using System.Data;

namespace Http.Service
{
    public class BulletinService
    {
        private readonly ConcurrentDictionary<(uint World, uint Section), ConcurrentQueue<BulletinWriteRequest>> _writeQueues = new();
        private readonly BulletinCacheService _cacheService;
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly LogService _logService;

        public BulletinService(BulletinCacheService cacheService, IServiceScopeFactory scopeFactory, LogService logService)
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

            var queue = _writeQueues.GetOrAdd((world, section), _ => new ConcurrentQueue<BulletinWriteRequest>());
            queue.Enqueue(request);

            return request.CompletionSource.Task;
        }

        public async Task<int> Delete(uint world, uint section, uint id, uint user, bool ignoreOwner = false)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            try
            {
                await using var conn = dbContext.GetShardConnection(world, section);

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
                    await _cacheService.DeleteArticlesBatchAsync(world, new List<(uint section, uint id)> { (section, id) });

                    await _logService.WriteAsync("bulletin_delete", new
                    {
                        world = world,
                        section = section,
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

        public async Task<int> DeleteBatch(uint world, uint section, List<uint> ids, uint user, bool ignoreOwner = false)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            try
            {
                await using var conn = dbContext.GetShardConnection(world, section);

                var deletedIds = new List<(uint section, uint id)>();
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
                        deletedIds.Add((section, id));
                        successCount++;
                    }
                }

                // Delete from Redis cache for all successfully deleted articles
                if (deletedIds.Any())
                {
                    await _cacheService.DeleteArticlesBatchAsync(world, deletedIds);

                    await _logService.WriteAsync("bulletin_delete_batch", new
                    {
                        world = world,
                        section = section,
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

        public async Task<int> Update(uint world, uint section, uint id, uint user, string title, string contents, bool ignoreOwner = false)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            try
            {
                await using var conn = dbContext.GetShardConnection(world, section);

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
                    var (article, _) = await LoadArticleFromDatabase(world, section, id);
                    if (article != null)
                    {
                        await _cacheService.SetArticleAsync(world, section, id, article);
                    }
                    else
                    {
                        await _cacheService.DeleteArticlesBatchAsync(world, new List<(uint section, uint id)> { (section, id) });
                    }

                    await _logService.WriteAsync("bulletin_update", new
                    {
                        world = world,
                        section = section,
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

        public Dictionary<uint, List<BulletinWriteRequest>> DequeueBatch(uint world, int maxBatchSize)
        {
            var writes = new Dictionary<uint, List<BulletinWriteRequest>>();
            foreach (var (key, queue) in _writeQueues)
            {
                if (key.World != world)
                    continue;

                var batch = new List<BulletinWriteRequest>();
                while (batch.Count < maxBatchSize && queue.TryDequeue(out var request))
                {
                    batch.Add(request);
                }
                if (batch.Count > 0)
                    writes[key.Section] = batch;
            }

            return writes;
        }

        public async Task<List<Bulletin>> GetArticleListAsync(
            uint world,
            uint section,
            ushort offset,
            string searchQuery = null,
            string searchField = "all")
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.GetShardConnection(world, section);

            List<Bulletin> articleList;

            if (string.IsNullOrWhiteSpace(searchQuery))
            {
                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("section", section);
                dynamicParams.Add("position", offset);

                var articles = await conn.QueryAsync<Bulletin>(
                    "USP_BULLETIN_GET_LIST",
                    dynamicParams,
                    commandType: CommandType.StoredProcedure);

                articleList = articles.ToList();
            }
            else
            {
                var searchPattern = $"%{searchQuery}%";
                uint? userId = null;

                if (searchField is "author" or "all")
                {
                    var foundUserId = await dbContext.Character.GetCharacterId(world, searchQuery);
                    if (foundUserId.HasValue)
                        userId = foundUserId.Value;
                }

                var sql = @"
                    SELECT b.id, b.section, b.user, b.title, b.contents, b.created_date, b.updated_date, b.deleted
                    FROM bulletin b
                    WHERE b.section = @section 
                      AND b.deleted = 0
                      AND @position >= b.id
                      AND (";

                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("section", section);
                dynamicParams.Add("search", searchPattern);
                dynamicParams.Add("position", offset);

                var conditions = new List<string>();
                if (searchField is "all" or "title")
                    conditions.Add("b.title LIKE @search");
                if (searchField is "all" or "contents")
                    conditions.Add("b.contents LIKE @search");
                if (searchField is "author" or "all")
                {
                    if (userId.HasValue)
                        conditions.Add("b.user = @userId");
                    else if (searchField == "author")
                        conditions.Add("1 = 0");
                }

                sql += string.Join(" OR ", conditions);
                sql += @")
                    ORDER BY b.id DESC
                    LIMIT 20";

                if (userId.HasValue)
                    dynamicParams.Add("userId", userId.Value);

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

        public async Task<(Bulletin Article, bool Next)> GetArticleAsync(uint world, uint section, uint id)
        {
            bool? nextFlagFromLoader = null;

            var article = await _cacheService.GetArticleAsync(
                world,
                section,
                id,
                async () =>
                {
                    var (dbArticle, next) = await LoadArticleFromDatabase(world, section, id);
                    nextFlagFromLoader = next;
                    return dbArticle;
                });

            if (article == null)
                return (null, false);

            var nextFlag = nextFlagFromLoader ?? await LoadNextFlagAsync(world, section, id);
            return (article, nextFlag);
        }

        private async Task<(Bulletin Article, bool Next)> LoadArticleFromDatabase(uint world, uint section, uint id)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.GetShardConnection(world, section);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("section", section);
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

        private async Task<bool> LoadNextFlagAsync(uint world, uint section, uint id)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.GetShardConnection(world, section);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("section", section);
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
