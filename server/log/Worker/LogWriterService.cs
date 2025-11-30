using Http.Service;
using Log.Repository;
using Microsoft.Extensions.DependencyInjection;
using StackExchange.Redis;
using System.Text.Json;

namespace Log.Worker
{
    /// <summary>
    /// Background service that periodically reads log entries from Redis and bulk inserts them into MySQL.
    /// Processes logs in batches with sharding support for both Redis and MySQL.
    /// </summary>
    public class LogWriterService : BackgroundService
    {
        private readonly RedisService _redisService;
        private readonly IServiceScopeFactory _serviceScopeFactory;
        private readonly ILogger<LogWriterService> _logger;
        private static readonly TimeSpan ProcessInterval = TimeSpan.FromSeconds(10);
        private const string LogListKeyPrefix = "log:pending:";
        private const int BatchSize = 1000;

        /// <summary>
        /// Initializes a new instance of the <see cref="LogWriterService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for reading logs.</param>
        /// <param name="serviceScopeFactory">The service scope factory for creating scoped dependencies.</param>
        /// <param name="logger">The logger instance.</param>
        public LogWriterService(
            RedisService redisService,
            IServiceScopeFactory serviceScopeFactory,
            ILogger<LogWriterService> logger)
        {
            _redisService = redisService;
            _serviceScopeFactory = serviceScopeFactory;
            _logger = logger;
        }

        /// <summary>
        /// Executes the background service logic.
        /// Periodically reads logs from Redis and writes them to MySQL in batches.
        /// </summary>
        /// <param name="stoppingToken">Cancellation token to stop the service.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            _logger.LogInformation("Log Writer Service started");

            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    await ProcessLogsAsync(stoppingToken);
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Error processing logs");
                }

                await Task.Delay(ProcessInterval, stoppingToken);
            }

            _logger.LogInformation("Log Writer Service stopped");
        }

        private const string LogListKey = "log:pending";

        /// <summary>
        /// Processes log entries from all Redis shards and writes them to MySQL.
        /// </summary>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        private async Task ProcessLogsAsync(CancellationToken cancellationToken)
        {
            var allLogs = new List<JsonElement>();

            // Scan all Redis shards for log lists
            for (int shardId = 0; shardId < _redisService.ShardSize; shardId++)
            {
                var redis = _redisService.Redis(shardId);
                if (redis == null)
                    continue;

                try
                {
                    // Read batch of logs from Redis list
                    var logStrings = await redis.Connection.ListRangeAsync(LogListKey, 0, BatchSize - 1);
                    
                    if (logStrings.Length == 0)
                        continue;

                    // Parse JSON strings to JsonElement
                    var logs = new List<JsonElement>();
                    foreach (var s in logStrings)
                    {
                        try
                        {
                            using var doc = JsonDocument.Parse(s.ToString());
                            logs.Add(doc.RootElement.Clone());
                        }
                        catch (Exception ex)
                        {
                            _logger.LogWarning(ex, $"Failed to parse log entry: {s}");
                        }
                    }

                    allLogs.AddRange(logs);

                    // Remove processed logs from Redis
                    await redis.Connection.ListLeftPopAsync(LogListKey, logStrings.Length);
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, $"Failed to process logs from Redis shard {shardId}");
                }
            }

            // Bulk insert all collected logs into MySQL
            if (allLogs.Count > 0)
            {
                try
                {
                    using var scope = _serviceScopeFactory.CreateScope();
                    var logRepository = scope.ServiceProvider.GetRequiredService<LogRepository>();
                    await logRepository.BulkInsertAsync(allLogs);
                    _logger.LogInformation($"Processed {allLogs.Count} log entries");
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, $"Failed to bulk insert {allLogs.Count} log entries");
                }
            }
        }
    }
}
