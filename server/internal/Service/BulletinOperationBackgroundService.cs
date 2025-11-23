using Dapper;
using Http.Extension;
using Http.Service;
using Microsoft.Extensions.DependencyInjection;

namespace Internal.Service
{
    /// <summary>
    /// Provides a background service that processes bulletin article write and delete operations in batches.
    /// Periodically dequeues accumulated requests from memory queues and executes them against the appropriate database shards.
    /// </summary>
    public class BulletinOperationBackgroundService : BackgroundService
    {
        private readonly BulletinOperationService _operationService;
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly ILogger<BulletinOperationBackgroundService> _logger;

        /// <summary>
        /// The time interval between batch processing cycles.
        /// </summary>
        private static readonly TimeSpan _processingInterval = TimeSpan.FromSeconds(5);

        /// <summary>
        /// The maximum number of requests to process per section in each batch.
        /// </summary>
        private static readonly int _maxBatchSize = 100;

        /// <summary>
        /// Initializes a new instance of the <see cref="BulletinOperationBackgroundService"/> class.
        /// </summary>
        /// <param name="operationService">The bulletin operation service for queue management.</param>
        /// <param name="scopeFactory">The service scope factory for creating scoped service instances.</param>
        /// <param name="logger">The logger for recording batch processing operations and errors.</param>
        public BulletinOperationBackgroundService(
            BulletinOperationService operationService,
            IServiceScopeFactory scopeFactory,
            ILogger<BulletinOperationBackgroundService> logger)
        {
            _operationService = operationService;
            _scopeFactory = scopeFactory;
            _logger = logger;
        }

        /// <summary>
        /// Executes the background service that processes bulletin operations in batches.
        /// Continuously processes accumulated write and delete requests at regular intervals.
        /// </summary>
        /// <param name="stoppingToken">The cancellation token for stopping the service.</param>
        /// <returns>A task representing the asynchronous execution of the background service.</returns>
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

        /// <summary>
        /// Processes a batch of write and delete requests from the operation service queues.
        /// </summary>
        /// <param name="cancellationToken">The cancellation token for stopping the operation.</param>
        /// <returns>A task representing the asynchronous batch processing.</returns>
        private async Task ProcessBatchAsync(CancellationToken cancellationToken)
        {
            var (writes, deletes) = _operationService.DequeueBatch(_maxBatchSize);

            // Process write requests
            await ProcessWritesAsync(writes, cancellationToken);

            // Process delete requests
            await ProcessDeletesAsync(deletes, cancellationToken);
        }

        /// <summary>
        /// Processes write requests grouped by section.
        /// Executes bulk INSERT operations with section-specific sequence management.
        /// </summary>
        /// <param name="writes">Dictionary mapping section IDs to their write request batches.</param>
        /// <param name="cancellationToken">The cancellation token for stopping the operation.</param>
        /// <returns>A task representing the asynchronous write processing.</returns>
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

                        // Notify success
                        foreach (var request in requests)
                        {
                            request.CompletionSource.SetResult(true);
                        }

                        _logger.LogInformation($"Processed {requests.Count} bulletin writes for section {section} (IDs: {startId}-{startId + requests.Count - 1})");
                    }
                    catch
                    {
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

        /// <summary>
        /// Processes delete requests grouped by section.
        /// Executes bulk UPDATE operations against the appropriate database shards.
        /// </summary>
        /// <param name="deletes">Dictionary mapping section IDs to their delete request batches.</param>
        /// <param name="cancellationToken">The cancellation token for stopping the operation.</param>
        /// <returns>A task representing the asynchronous delete processing.</returns>
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
                    await conn.ExecuteAsync(sql);

                    // Notify success
                    // Note: Bulk UPDATE does not return individual results, so we treat all as successful
                    // In practice, individual result verification would require separate queries,
                    // but we simplify for batch processing benefits
                    foreach (var request in requests)
                    {
                        request.CompletionSource.SetResult(1);
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

        /// <summary>
        /// Determines the database shard index for a given section using modular sharding.
        /// </summary>
        /// <param name="section">The section identifier to determine the shard for.</param>
        /// <param name="dbContext">The database context to get SharedDbSize from.</param>
        /// <returns>The database index to use for the section. Returns -1 if SharedDbSize is zero (common DB).</returns>
        private int GetDbIndexForSection(uint section, DbContext dbContext)
        {
            // Modular sharding: section % SharedDbSize
            // If SharedDbSize is 0, use -1 (common DB)
            if (dbContext.SharedDbSize == 0)
                return -1;

            return (int)(section % dbContext.SharedDbSize);
        }

        /// <summary>
        /// Builds a bulk INSERT SQL query for multiple write requests with assigned IDs.
        /// Uses MySql.Escape to prevent SQL injection.
        /// </summary>
        /// <param name="requests">The list of write requests to include in the bulk insert.</param>
        /// <param name="section">The section identifier for all requests.</param>
        /// <param name="startId">The starting ID to assign to the first request.</param>
        /// <returns>A SQL INSERT statement with multiple value rows including assigned IDs.</returns>
        private string BuildBulkInsertQuery(List<BulletinWriteRequest> requests, uint section, uint startId)
        {
            // Use MySql.Escape for SQL injection prevention
            // Assign sequential IDs starting from startId
            var values = string.Join(", ", requests.Select((r, index) =>
                $"({startId + index}, {section}, {r.User}, {r.Title.Escape()}, {r.Contents.Escape()})"));

            return $"INSERT INTO bulletin (`id`, `section`, `user`, title, contents) VALUES {values}";
        }

        /// <summary>
        /// Builds a bulk UPDATE SQL query for multiple delete requests.
        /// Uses (id, user) combinations to create WHERE conditions.
        /// </summary>
        /// <param name="requests">The list of delete requests to include in the bulk update.</param>
        /// <returns>A SQL UPDATE statement with multiple WHERE conditions.</returns>
        private string BuildBulkDeleteQuery(List<BulletinDeleteRequest> requests)
        {
            // Create WHERE conditions using (id, user) combinations
            var conditions = string.Join(" OR ", requests.Select(r =>
                $"(id = {r.Id} AND `user` = {r.User})"));

            return $"UPDATE bulletin SET deleted = 1, updated_date = NOW() WHERE {conditions}";
        }
    }
}

