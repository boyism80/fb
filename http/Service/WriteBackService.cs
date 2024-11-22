using Dapper;
using Fb.Model.EnumValue;
using Http.Redis;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    public class BackgroundCommitEntry
    {
        public required string SQL { get; set; }
        public required string RedisKey { get; set; }
    };

    public class WriteBackService : BackgroundService
    {
        private readonly RedisService _redisService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<WriteBackService> _logger;
        private const string RedisBufferKey = "exec-buffer";
        private static readonly TimeSpan _delay = TimeSpan.FromMilliseconds(500);

        public WriteBackService(RedisService redisService,
            IConfiguration configuration,
            IServiceProvider serviceProvider,
            ILogger<WriteBackService> logger)
        {
            _redisService = redisService;
            _configuration = configuration;
            _logger = logger;
            _dbContext = ActivatorUtilities.CreateInstance<DbContext>(serviceProvider);
        }

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
                thread.Start();
            }

            while (!stoppingToken.IsCancellationRequested && threads.Any(thread => thread.IsAlive))
            {
                await Task.Delay(_delay, stoppingToken);
            }
        }

        private async Task OnWork(int db, CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    var connRedis = _redisService.Connection;
                    var success = await connRedis.TrySync($"write-back-{db}", async () =>
                    {
                        var result = await connRedis.ScriptEvaluateAsync("pop_sql_range.lua", new
                        {
                            key = new RedisKey($"{(RedisBufferKey)}:{db}"),
                            count = 100
                        });
                        if (result.Length == 0)
                            return false;

                        await using var connection = _dbContext.Connection(db);
                        var backgroundCommitEntryList = ((RedisResult[])result).Select((Func<RedisResult, BackgroundCommitEntry>)(x => (BackgroundCommitEntry)JsonConvert.DeserializeObject<BackgroundCommitEntry>(x.ToString())));
                        foreach (var g in backgroundCommitEntryList.GroupBy(x => x.RedisKey))
                        {
                            var redisKey = g.Key;
                            var sql = string.Join(Environment.NewLine, g.Select(x => x.SQL));
                            await connection.ExecuteAsync(sql);

                            await connRedis.ScriptEvaluateAsync("end_of_ref.lua", new
                            {
                                key = new RedisKey(Http.Redis.Const.ReferenceCountKey),
                                field = new RedisValue(redisKey),
                                count = g.Count(),
                                expiry = (int)Http.Redis.Const.CacheTimeToLive.TotalSeconds,
                            });
                        }
                        return true;
                    });

                    if (!success)
                        await Task.Delay(_delay, stoppingToken);
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

        public async Task Post(int db, string sql, string key)
        {
            var bufferKey = $"{RedisBufferKey}:{db}";
            await _redisService.Connection.ListRightPushAsync(bufferKey, JsonConvert.SerializeObject(new BackgroundCommitEntry
            {
                SQL = sql,
                RedisKey = key
            }));
        }

        public async Task Post(uint modKey, string sql, string key)
        {
            var db = modKey % _dbContext.SharedDbSize;
            await Post((int)db, sql, key);
        }
    }
}
