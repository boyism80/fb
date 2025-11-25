using Dapper;
using Http.Extension;
using Http.Service;
using Microsoft.Extensions.DependencyInjection;

namespace Internal.Service
{
    public class BulletinBackgroundService : BackgroundService
    {
        private readonly BulletinService _bulletinService;
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly ILogger<BulletinBackgroundService> _logger;
        private readonly BulletinCacheService _bulletinCacheService;

        private static readonly TimeSpan _processingInterval = TimeSpan.FromSeconds(1);

        private static readonly int _maxBatchSize = 100;

        public BulletinBackgroundService(
            BulletinService bulletinService,
            IServiceScopeFactory scopeFactory,
            ILogger<BulletinBackgroundService> logger,
            BulletinCacheService bulletinCacheService)
        {
            _bulletinService = bulletinService;
            _scopeFactory = scopeFactory;
            _logger = logger;
            _bulletinCacheService = bulletinCacheService;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    await ProcessBatchAsync(stoppingToken);
                    await Task.Delay(_processingInterval, stoppingToken);
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Error processing bulletin operation batch");
                    await Task.Delay(_processingInterval, stoppingToken);
                }
            }
        }

        private async Task ProcessBatchAsync(CancellationToken cancellationToken)
        {
            var (writes, deletes) = _bulletinService.DequeueBatch(_maxBatchSize);

            // Process write requests
            await ProcessWritesAsync(writes, cancellationToken);

            // Process delete requests
            await ProcessDeletesAsync(deletes, cancellationToken);
        }

        private async Task ProcessWritesAsync(Dictionary<uint, List<BulletinWriteRequest>> writes, CancellationToken cancellationToken)
        {
            // Group by section and process
            foreach (var (section, requests) in writes)
            {
                using var scope = _scopeFactory.CreateScope();
                var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

                try
                {
                    // Modular sharding: section % SharedDbSize
                    var dbIndex = GetDbIndexForSection(section, dbContext);

                    await using var conn = dbContext.Connection(dbIndex);
                    await conn.OpenAsync(cancellationToken);

                    // Start transaction for sequence management
                    await using var transaction = await conn.BeginTransactionAsync(cancellationToken);

                    try
                    {
                        // Get current sequence for this section (with lock)
                        var currentSequence = await conn.QueryFirstOrDefaultAsync<uint?>(
                            "SELECT id FROM bulletin_sequence WHERE `section` = @section FOR UPDATE",
                            new { section },
                            transaction);

                        uint startId;
                        if (currentSequence == null)
                        {
                            // First articles for this section
                            startId = 1;
                            await conn.ExecuteAsync(
                                "INSERT INTO bulletin_sequence (`section`, `id`) VALUES (@section, @id)",
                                new { section, id = (uint)(startId + requests.Count - 1) },
                                transaction);
                        }
                        else
                        {
                            // Increment sequence by request count
                            startId = currentSequence.Value + 1;
                            await conn.ExecuteAsync(
                                "UPDATE bulletin_sequence SET id = @id WHERE `section` = @section",
                                new { section, id = (uint)(startId + requests.Count - 1) },
                                transaction);
                        }

                        // Build bulk INSERT query with assigned IDs
                        var sql = BuildBulkInsertQuery(requests, section, startId);
                        await conn.ExecuteAsync(sql, transaction: transaction);

                        // Commit transaction
                        await transaction.CommitAsync(cancellationToken);

                        // Cache articles in Redis after successful DB insert
                        var cacheItems = new Dictionary<(uint section, uint id), Http.Model.Bulletin>();
                        for (int i = 0; i < requests.Count; i++)
                        {
                            var articleId = startId + (uint)i;
                            var request = requests[i];

                            var article = new Http.Model.Bulletin
                            {
                                Id = articleId,
                                Section = section,
                                User = request.User,
                                Title = request.Title,
                                Contents = request.Contents,
                                CreatedDate = DateTime.Now,
                                UpdatedDate = DateTime.Now,
                                Deleted = false
                            };

                            cacheItems[(section, articleId)] = article;
                        }

                        // Batch cache insert
                        await _bulletinCacheService.SetArticlesBatchAsync(cacheItems);

                        // Notify success
                        foreach (var request in requests)
                        {
                            request.CompletionSource.SetResult(true);
                        }

                        _logger.LogInformation($"Processed {requests.Count} bulletin writes for section {section} (IDs: {startId}-{startId + requests.Count - 1})");
                    }
                    catch (Exception e)
                    {
                        _logger.LogError(e, $"Error during write transaction for section {section}");
                        await transaction.RollbackAsync(cancellationToken);
                        throw;
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, $"Error processing write batch for section {section}");

                    // Notify failure
                    foreach (var request in requests)
                    {
                        request.CompletionSource.SetResult(false);
                    }
                }
            }
        }

        private async Task ProcessDeletesAsync(Dictionary<uint, List<BulletinDeleteRequest>> deletes, CancellationToken cancellationToken)
        {
            // Group by section and process (same approach as writes)
            foreach (var (section, requests) in deletes)
            {
                using var scope = _scopeFactory.CreateScope();
                var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

                try
                {
                    // Modular sharding: section % SharedDbSize
                    var dbIndex = GetDbIndexForSection(section, dbContext);

                    await using var conn = dbContext.Connection(dbIndex);

                    // Build bulk UPDATE query
                    var sql = BuildBulkDeleteQuery(requests);
                    var affectedRows = await conn.ExecuteAsync(sql);

                    // Delete from Redis cache
                    var articleKeys = requests.Select(r => (section, r.Id)).ToList();
                    await _bulletinCacheService.DeleteArticlesBatchAsync(articleKeys);

                    // Check if all requests were processed successfully
                    // If affectedRows < requests.Count, some requests targeted non-existent data
                    if (affectedRows < requests.Count)
                    {
                        // Some requests failed (non-existent data), mark all as failed
                        foreach (var request in requests)
                        {
                            request.CompletionSource.SetResult(-1);
                        }
                    }
                    else
                    {
                        // All requests succeeded
                        foreach (var request in requests)
                        {
                            request.CompletionSource.SetResult(1);
                        }
                    }

                    _logger.LogInformation($"Processed {requests.Count} bulletin deletes for section {section}");
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, $"Error processing delete batch for section {section}");

                    // Notify failure
                    foreach (var request in requests)
                    {
                        request.CompletionSource.SetResult(-4);
                    }
                }
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

        private string BuildBulkInsertQuery(List<BulletinWriteRequest> requests, uint section, uint startId)
        {
            // Use MySql.Escape for SQL injection prevention
            // Assign sequential IDs starting from startId
            var values = string.Join(", ", requests.Select((r, index) =>
                $"({startId + index}, {section}, {r.User}, {r.Title.Escape()}, {r.Contents.Escape()})"));

            return $"INSERT INTO bulletin (`id`, `section`, `user`, title, contents) VALUES {values}";
        }

        private string BuildBulkDeleteQuery(List<BulletinDeleteRequest> requests)
        {
            // Create WHERE conditions using (id, user) combinations
            var conditions = string.Join(" OR ", requests.Select(r =>
                $"(id = {r.Id} AND `user` = {r.User})"));

            return $"UPDATE bulletin SET deleted = 1, updated_date = NOW() WHERE deleted = 0 AND ({conditions})";
        }
    }
}

