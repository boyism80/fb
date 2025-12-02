using Dapper;
using Http.Extension;
using System.Data;

namespace Http.Service
{
    public class BulletinBackgroundService : BackgroundService
    {
        private readonly BulletinService _bulletinService;
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly ILogger<BulletinBackgroundService> _logger;
        private readonly BulletinCacheService _bulletinCacheService;
        private readonly LogService? _logService;

        private static readonly TimeSpan _processingInterval = TimeSpan.FromSeconds(1);

        private static readonly int _maxBatchSize = 100;

        public BulletinBackgroundService(
            BulletinService bulletinService,
            IServiceScopeFactory scopeFactory,
            ILogger<BulletinBackgroundService> logger,
            BulletinCacheService bulletinCacheService,
            LogService? logService = null)
        {
            _bulletinService = bulletinService;
            _scopeFactory = scopeFactory;
            _logger = logger;
            _bulletinCacheService = bulletinCacheService;
            _logService = logService;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    var writes = _bulletinService.DequeueBatch(_maxBatchSize);
                    await ProcessWritesAsync(writes, stoppingToken);
                    await Task.Delay(_processingInterval, stoppingToken);
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Error processing bulletin operation batch");
                    await Task.Delay(_processingInterval, stoppingToken);
                }
            }
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
                    await using var conn = dbContext.Connection(section);
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
                        var names = await dbContext.Character.GetName(requests.Select(r => r.User));
                        for (int i = 0; i < requests.Count; i++)
                        {
                            var articleId = startId + (uint)i;
                            var request = requests[i];

                            var article = new Http.Model.Bulletin
                            {
                                Id = articleId,
                                Section = section,
                                User = request.User,
                                UserName = names.GetValueOrDefault(request.User, string.Empty),
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

                        // Log bulletin write events
                        foreach (var request in requests)
                        {
                            var articleId = startId + (uint)requests.IndexOf(request);
                            _logService?.Write("bulletin_write", new
                            {
                                section = section,
                                article_id = articleId,
                                user_id = request.User
                            });
                        }

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

        private string BuildBulkInsertQuery(List<BulletinWriteRequest> requests, uint section, uint startId)
        {
            // Use MySql.Escape for SQL injection prevention
            // Assign sequential IDs starting from startId
            var values = string.Join(", ", requests.Select((r, index) =>
                $"({startId + index}, {section}, {r.User}, {r.Title.Escape()}, {r.Contents.Escape()})"));

            return $"INSERT INTO bulletin (`id`, `section`, `user`, title, contents) VALUES {values}";
        }

    }
}

