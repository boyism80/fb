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
        private const string RedisBufferKey = "exec-buffer";
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
            var bufferKey = $"{RedisBufferKey}:{db}";
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
                        continue;

                    await using var dbConn = _dbContext.Connection(db);
                    var backgroundCommitEntryList = ((RedisResult[])result).Select((x => JsonConvert.DeserializeObject<BackgroundCommitEntry>(x.ToString())));
                    foreach (var g in backgroundCommitEntryList.GroupBy(x => x.RedisKey))
                    {
                        var sql = string.Join(Environment.NewLine, g.Select(x => x.SQL));
                        await dbConn.ExecuteAsync(sql);

                        await _redisService.Redis(g.Key).ScriptEvaluateAsync("end_of_ref.lua", new
                        {
                            key = new RedisKey(Http.Redis.Const.ReferenceCountKey),
                            field = new RedisValue(g.Key),
                            count = g.Count(),
                            expiry = (int)Http.Redis.Const.CacheTimeToLive.TotalSeconds,
                        });
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
