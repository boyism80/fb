using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Data;
using System.Threading.Tasks;
using Dapper;
using Http.Model;
using Http.Service;
using Microsoft.Extensions.DependencyInjection;

namespace Internal.Service
{
    public class BulletinService
    {
        private readonly ConcurrentDictionary<uint, ConcurrentQueue<BulletinWriteRequest>> _writeQueues = new();
        private readonly BulletinCacheService _cacheService;
        private readonly IServiceScopeFactory _scopeFactory;

        public BulletinService(BulletinCacheService cacheService, IServiceScopeFactory scopeFactory)
        {
            _cacheService = cacheService;
            _scopeFactory = scopeFactory;
        }

        public Task<bool> Write(uint section, uint user, string title, string contents)
        {
            var request = new BulletinWriteRequest
            {
                Section = section,
                User = user,
                Title = title,
                Contents = contents,
                CompletionSource = new TaskCompletionSource<bool>()
            };

            var queue = _writeQueues.GetOrAdd(section, _ => new ConcurrentQueue<BulletinWriteRequest>());
            queue.Enqueue(request);

            return request.CompletionSource.Task;
        }

        public async Task<int> Delete(uint section, uint id, uint user, bool ignoreOwner = false)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            try
            {
                // Modular sharding: section % SharedDbSize
                var dbIndex = GetDbIndexForSection(section, dbContext);

                await using var conn = dbContext.Connection(dbIndex);
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
                    await _cacheService.DeleteArticlesBatchAsync(new List<(uint section, uint id)> { (section, id) });
                }

                return result;
            }
            catch (Exception)
            {
                // Log error and return failure code
                return -4;
            }
        }

        public async Task<int> Update(uint section, uint id, uint user, string title, string contents, bool ignoreOwner = false)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            try
            {
                var dbIndex = GetDbIndexForSection(section, dbContext);

                await using var conn = dbContext.Connection(dbIndex);
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
                    var (article, _) = await LoadArticleFromDatabase(section, id);
                    if (article != null)
                    {
                        await _cacheService.SetArticleAsync(section, id, article);
                    }
                    else
                    {
                        await _cacheService.DeleteArticlesBatchAsync(new List<(uint section, uint id)> { (section, id) });
                    }
                }

                return result;
            }
            catch (Exception)
            {
                return -4;
            }
        }

        private int GetDbIndexForSection(uint section, DbContext dbContext)
        {
            // Modular sharding: section % SharedDbSize
            // If SharedDbSize is 0, use -1 (common DB)
            if (dbContext.SharedDbSize == 0)
                return -1;

            return (int)(section % dbContext.SharedDbSize);
        }

        public Dictionary<uint, List<BulletinWriteRequest>> DequeueBatch(int maxBatchSize)
        {
            var writes = new Dictionary<uint, List<BulletinWriteRequest>>();
            foreach (var (section, queue) in _writeQueues)
            {
                var batch = new List<BulletinWriteRequest>();
                while (batch.Count < maxBatchSize && queue.TryDequeue(out var request))
                {
                    batch.Add(request);
                }
                if (batch.Count > 0)
                    writes[section] = batch;
            }

            return writes;
        }

        public async Task<(Bulletin Article, bool Next)> GetArticleAsync(uint section, uint id)
        {
            bool? nextFlagFromLoader = null;

            var article = await _cacheService.GetArticleAsync(
                section,
                id,
                async () =>
                {
                    var (dbArticle, next) = await LoadArticleFromDatabase(section, id);
                    nextFlagFromLoader = next;
                    return dbArticle;
                });

            if (article == null)
                return (null, false);

            var nextFlag = nextFlagFromLoader ?? await LoadNextFlagAsync(section, id);
            return (article, nextFlag);
        }

        private async Task<(Bulletin Article, bool Next)> LoadArticleFromDatabase(uint section, uint id)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.Connection(section);
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

            dbArticle.UserName = await dbContext.Character.GetName(dbArticle.User) ?? string.Empty;
            var next = await reader.ReadFirstAsync<bool>();

            return (dbArticle, next);
        }

        private async Task<bool> LoadNextFlagAsync(uint section, uint id)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

            await using var conn = dbContext.Connection(section);
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
