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
        private readonly ConcurrentDictionary<uint, ConcurrentQueue<BulletinDeleteRequest>> _deleteQueues = new();
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

        public Task<int> Delete(uint section, uint id, uint user)
        {
            var request = new BulletinDeleteRequest
            {
                Section = section,
                Id = id,
                User = user,
                CompletionSource = new TaskCompletionSource<int>()
            };

            var queue = _deleteQueues.GetOrAdd(section, _ => new ConcurrentQueue<BulletinDeleteRequest>());
            queue.Enqueue(request);

            return request.CompletionSource.Task;
        }

        public (Dictionary<uint, List<BulletinWriteRequest>> Writes, Dictionary<uint, List<BulletinDeleteRequest>> Deletes) DequeueBatch(int maxBatchSize)
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

            var deletes = new Dictionary<uint, List<BulletinDeleteRequest>>();
            foreach (var (section, queue) in _deleteQueues)
            {
                var batch = new List<BulletinDeleteRequest>();
                while (batch.Count < maxBatchSize && queue.TryDequeue(out var request))
                {
                    batch.Add(request);
                }
                if (batch.Count > 0)
                    deletes[section] = batch;
            }

            return (writes, deletes);
        }

        public async Task<(Bulletin? Article, bool Next)> GetArticleAsync(uint section, ushort id)
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

        private async Task<(Bulletin? Article, bool Next)> LoadArticleFromDatabase(uint section, ushort id)
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

        private async Task<bool> LoadNextFlagAsync(uint section, ushort id)
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
