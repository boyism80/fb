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
        }

        /// <summary>
        /// Executes the background service that processes write-back operations.
        /// Creates worker threads for each database shard and monitors their execution.
        /// </summary>
        /// <param name="stoppingToken">The cancellation token for stopping the service.</param>
        /// <returns>A task representing the asynchronous execution of the background service.</returns>
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var section = _configuration.GetSection("ConnectionStrings:MySql");
            var threads = section.GetChildren().Select(x => new Thread(() =>
            {
                var task = OnWork(int.Parse(x.Key), stoppingToken);
                task.Wait(stoppingToken);
            })).ToArray();

            foreach (var thread in threads)
            {
                thread.Name = $"WriteBackThread";
                thread.Start();
            }

            while (!stoppingToken.IsCancellationRequested && threads.Any(thread => thread.IsAlive))
            {
                await Task.Delay(_delay, stoppingToken);
            }
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
            while (continuous || !stoppingToken.IsCancellationRequested)
            {
                try
                {
                    var redisSqlConn = _redisService.Redis(bufferKey);
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

                    await using var dbConn = _dbContext.Connection(db);
                    var backgroundCommitEntryList = ((RedisResult[])result).Select((x => JsonConvert.DeserializeObject<BackgroundCommitEntry>(x.ToString())));
                    foreach (var g in backgroundCommitEntryList.GroupBy(x => (int)(x.Hash % _redisService.ShardSize)))
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

                        await _redisService.Redis(mod).ScriptEvaluateAsync("end_of_ref.lua",
                            keys: [new RedisKey(Const.ReferenceCountKey)],
                            values: [.. values]);
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
