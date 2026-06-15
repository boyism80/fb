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
        private readonly LogService _logService;
        private readonly IConfiguration _configuration;

        private static readonly TimeSpan _processingInterval = TimeSpan.FromSeconds(1);

        private static readonly int _maxBatchSize = 100;

        public BulletinBackgroundService(
            BulletinService bulletinService,
            IServiceScopeFactory scopeFactory,
            ILogger<BulletinBackgroundService> logger,
            BulletinCacheService bulletinCacheService,
            IConfiguration configuration,
            LogService logService)
        {
            _bulletinService = bulletinService;
            _scopeFactory = scopeFactory;
            _logger = logger;
            _bulletinCacheService = bulletinCacheService;
            _configuration = configuration;
            _logService = logService;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            // Get world keys: DbContext uses ConnectionStrings:MySql:worlds:{world}:...
            var worldKeys = _configuration.GetSection("ConnectionStrings:MySql:worlds")
                .GetChildren()
                .Select(x => x.Key)
                .ToList();

            while (!stoppingToken.IsCancellationRequested)
            {
                foreach (var worldKey in worldKeys)
                {
                    try
                    {
                        // Parse world key: "1", "2", etc. (must be numeric; bulletin uses per-world DB)
                        if (!uint.TryParse(worldKey, out var world) || world == 0)
                            continue;

                        var writes = _bulletinService.DequeueBatch(world, _maxBatchSize);
                        await ProcessWritesAsync(world, writes, stoppingToken);
                        await Task.Delay(_processingInterval, stoppingToken);
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError(ex, "Error processing bulletin operation batch for world {WorldKey}", worldKey);
                    }
                }
                await Task.Delay(_processingInterval, stoppingToken);
            }
        }

        private async Task ProcessWritesAsync(uint world, Dictionary<uint, List<BulletinWriteRequest>> writes, CancellationToken cancellationToken)
        {
            // Group by section and process
            foreach (var (bulletinSection, requests) in writes)
            {
                using var scope = _scopeFactory.CreateScope();
                var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();

                try
                {
                    await using var conn = dbContext.GetShardConnection(world, bulletinSection);
                    // Connection must be open before BeginTransactionAsync (Dapper does not open for transactions).
                    await conn.OpenAsync(cancellationToken);

                    // Start transaction for sequence management
                    await using var transaction = await conn.BeginTransactionAsync(cancellationToken);

                    try
                    {
                        // Get current sequence for this bulletin section (with lock)
                        // Note: bulletinSection is the bulletin category, not world
                        var currentSequence = await conn.QueryFirstOrDefaultAsync<uint?>(
                            "SELECT id FROM bulletin_sequence WHERE `section` = @section FOR UPDATE",
                            new { section = bulletinSection },
                            transaction);

                        uint startId;
                        if (currentSequence == null)
                        {
                            // First articles for this bulletin section
                            startId = 1;
                            await conn.ExecuteAsync(
                                "INSERT INTO bulletin_sequence (`section`, `id`) VALUES (@section, @id)",
                                new { section = bulletinSection, id = (uint)(startId + requests.Count - 1) },
                                transaction);
                        }
                        else
                        {
                            // Increment sequence by request count
                            startId = currentSequence.Value + 1;
                            await conn.ExecuteAsync(
                                "UPDATE bulletin_sequence SET id = @id WHERE `section` = @section",
                                new { section = bulletinSection, id = (uint)(startId + requests.Count - 1) },
                                transaction);
                        }

                        // Build bulk INSERT query with assigned IDs
                        var sql = BuildBulkInsertQuery(requests, bulletinSection, startId);
                        await conn.ExecuteAsync(sql, transaction: transaction);

                        // Commit transaction
                        await transaction.CommitAsync(cancellationToken);

                        // Cache articles in Redis after successful DB insert
                        var cacheItems = new Dictionary<(uint section, uint id), Http.Model.Bulletin>();
                        var names = await dbContext.Character.GetName(world, requests.Select(r => r.User));
                        for (int i = 0; i < requests.Count; i++)
                        {
                            var articleId = startId + (uint)i;
                            var request = requests[i];

                            var article = new Http.Model.Bulletin
                            {
                                Id = articleId,
                                Section = bulletinSection,
                                User = request.User,
                                UserName = names.GetValueOrDefault(request.User, string.Empty),
                                Title = request.Title,
                                Contents = request.Contents,
                                CreatedDate = DateTime.Now,
                                UpdatedDate = DateTime.Now,
                            };

                            cacheItems[(bulletinSection, articleId)] = article;
                        }

                        // Batch cache insert
                        await _bulletinCacheService.SetArticlesBatchAsync(world, cacheItems);

                        for (int i = 0; i < requests.Count; i++)
                        {
                            var request = requests[i];
                            var articleId = startId + (uint)i;
                            await _logService.WriteAsync("bulletin_write", new
                            {
                                world = world,
                                bulletinSection = bulletinSection,
                                article_id = articleId,
                                user_id = request.User
                            });
                        }

                        // Notify success
                        foreach (var request in requests)
                        {
                            request.CompletionSource.SetResult(true);
                        }

                        _logger.LogInformation($"Processed {requests.Count} bulletin writes for section {bulletinSection} (IDs: {startId}-{startId + requests.Count - 1})");
                    }
                    catch (Exception e)
                    {
                        _logger.LogError(e, $"Error during write transaction for section {bulletinSection}");
                        await transaction.RollbackAsync(cancellationToken);
                        throw;
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, $"Error processing write batch for section {bulletinSection}");

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

