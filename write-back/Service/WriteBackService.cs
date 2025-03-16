using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Redis;
using Http.Service;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace WriteBack.Service
{
    public class WriteBackService
    {
        private readonly RedisService _redisService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<Http.Service.WriteBackService> _logger;
        private static readonly TimeSpan _delay = TimeSpan.FromSeconds(5);

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

        public async Task ExecuteAsync(CancellationToken stoppingToken)
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

        private async Task OnWork(int db, CancellationToken stoppingToken)
        {
            var bufferKey = $"{Const.RedisBufferKey}:{db}";
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    var redisSqlConn = _redisService.Redis(bufferKey);
                    var result = await redisSqlConn.ScriptEvaluateAsync("pop_sql_range.lua", new
                    {
                        key = new RedisKey(bufferKey),
                        count = 100
                    });
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
