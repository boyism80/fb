using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Redis;
using Http.Service;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace WriteBack.Service
{
    /// <summary>
    /// Provides a background service that processes deferred database write operations.
    /// Consumes SQL operations from Redis queues and executes them against the appropriate database shards.
    /// </summary>
    public class WriteBackService : BackgroundService
    {
        private readonly RedisService _redisService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<Http.Service.WriteBackService> _logger;
        private readonly uint _world;
        private static readonly TimeSpan _delay = TimeSpan.FromSeconds(5);

        /// <summary>
        /// Initializes a new instance of the <see cref="WriteBackService"/> class.
        /// </summary>
        /// <param name="redisService">The Redis service for accessing queue operations.</param>
        /// <param name="configuration">The application configuration containing database connection strings.</param>
        /// <param name="serviceProvider">The service provider for dependency injection.</param>
        /// <param name="logger">The logger for recording write-back operations and errors.</param>
        public WriteBackService(RedisService redisService,
            IConfiguration configuration,
            IServiceProvider serviceProvider,
            ILogger<Http.Service.WriteBackService> logger)
        {
            _redisService = redisService;
            _configuration = configuration;
            _logger = logger;
            _dbContext = ActivatorUtilities.CreateInstance<DbContext>(serviceProvider);
            _world = _configuration.GetValue<uint>("World");
            if (_world == 0 && _configuration["World"] != "0")
                throw new Exception("World configuration is required");
        }

        /// <summary>
        /// Executes the background service that processes write-back operations.
        /// Creates worker threads for each database shard and monitors their execution.
        /// </summary>
        /// <param name="stoppingToken">The cancellation token for stopping the service.</param>
        /// <returns>A task representing the asynchronous execution of the background service.</returns>
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            if (_world == 0)
            {
                throw new Exception("Write-back service requires a world > 0. Unified-global is not supported.");
            }

            var worldKey = _world.ToString();
            var mysqlSection = _configuration.GetSection($"ConnectionStrings:MySql:worlds:{worldKey}");
            if (!mysqlSection.Exists())
            {
                throw new Exception($"MySQL configuration not found for world: {_world}");
            }

            var threadList = new List<Thread>();

            // Add thread for global database (-1)
            var globalSection = mysqlSection.GetSection("global");
            if (globalSection.Exists())
            {
                threadList.Add(new Thread(() =>
                {
                    var task = OnWork(-1, stoppingToken);
                    try
                    {
                        task.Wait(stoppingToken);
                    }
                    catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
                    {
                        _logger.LogInformation("Write-back worker for world {World} DB {DbId} cancelled.", _world, -1);
                    }
                }));
            }

            // Add threads for data shards (0, 1, 2, ...)
            var dataArray = mysqlSection.GetSection("data").Get<string[]>();
            if (dataArray != null && dataArray.Length > 0)
            {
                for (int i = 0; i < dataArray.Length; i++)
                {
                    var shardIndex = i; // Capture loop variable
                    threadList.Add(new Thread(() =>
                    {
                        var task = OnWork(shardIndex, stoppingToken);
                        try
                        {
                            task.Wait(stoppingToken);
                        }
                        catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
                        {
                            _logger.LogInformation("Write-back worker for world {World} DB {DbId} cancelled.", _world, shardIndex);
                        }
                    }));
                }
            }

            var threads = threadList.ToArray();

            foreach (var thread in threads)
            {
                thread.Name = $"WriteBackThread";
                thread.Start();
            }

            while (!stoppingToken.IsCancellationRequested && threads.Any(thread => thread.IsAlive))
            {
                await Task.Delay(_delay, stoppingToken);
            }

            _logger.LogInformation("All write-back workers stopped for world {World}, all Redis data processed", _world);
        }

        /// <summary>
        /// Processes write-back operations for a specific database shard.
        /// Continuously polls Redis queues for pending SQL operations and executes them in batches.
        /// </summary>
        /// <param name="db">The database shard identifier to process operations for.</param>
        /// <param name="stoppingToken">The cancellation token for stopping the worker.</param>
        /// <returns>A task representing the asynchronous processing of write-back operations.</returns>
        private async Task OnWork(int db, CancellationToken stoppingToken)
        {
            var bufferKey = $"{Const.RedisBufferKey}:{db}";
            var continuous = true;
            var bulk = 100;
            var shardSize = _redisService.GetShardSize(_world);
            while (continuous || !stoppingToken.IsCancellationRequested)
            {
                try
                {
                    var redisSqlConn = db == -1 ? _redisService.GetGlobalConnection(_world) : _redisService.GetDataConnection(_world, db);
                    if (redisSqlConn == null)
                    {
                        _logger.LogError("Redis connection not found for world {World} DB {Db}", _world, db);
                        await Task.Delay(_delay, stoppingToken);
                        continue;
                    }

                    var result = await redisSqlConn.ScriptEvaluateAsync("pop_sql_range.lua", new
                    {
                        key = new RedisKey(bufferKey),
                        count = bulk
                    });
                    continuous = result.Length == bulk;

                    if (result.Length == 0)
                    {
                        await Task.Delay(_delay, stoppingToken);
                        continue;
                    }

                    await using var dbConn = db == -1 ? _dbContext.GetGlobalConnection(_world) : _dbContext.GetDataConnection(_world, db);
                    var backgroundCommitEntryList = ((RedisResult[])result).Select((x => JsonConvert.DeserializeObject<BackgroundCommitEntry>(x.ToString())));
                    foreach (var g in backgroundCommitEntryList.GroupBy(x => x.Hash != null ? (int)(x.Hash % shardSize) : -1))
                    {
                        var mod = g.Key;
                        var sql = string.Join(Environment.NewLine, g.Select(x => x.SQL));
                        await dbConn.ExecuteAsync(sql);
                        _logger.LogInformation(sql);

                        var countSet = g.GroupBy(x => x.RedisKey).ToDictionary(x => x.Key, x => x.Count());
                        var values = new List<RedisValue>
                        {
                            (int)Http.Redis.Const.CacheTimeToLive.TotalSeconds,
                            countSet.Count,
                        };
                        foreach (var (field, count) in countSet)
                        {
                            values.Add(field);
                            values.Add(count);
                        }

                        var redisRefConn = mod == -1 ? _redisService.GetGlobalConnection(_world) : _redisService.GetDataConnection(_world, mod);
                        if (redisRefConn != null)
                        {
                            await redisRefConn.ScriptEvaluateAsync("end_of_ref.lua",
                                keys: [new RedisKey(Const.ReferenceCountKey)],
                                values: [.. values]);
                        }
                    }
                }
                catch (LogicException e)
                {
                    switch (e.Error)
                    {
                        case ErrorCode.DistributedLockFailed:
                            await Task.Delay(_delay, stoppingToken);
                            break;

                        default:
                            _logger.LogError(e, e.Message);
                            break;
                    }
                }
                catch (Exception e)
                {
                    _logger.LogError(e, e.Message);
                }
            }
        }
    }
}
