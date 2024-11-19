using Dapper;
using http.Redis;
using http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Db.Service
{
    public class BackgroundCommitEntry
    {
        public required string SQL { get; set; }
        public required string RedisKey { get; set; }
    };

    public class DbExecuteService : BackgroundService
    {
        private readonly RedisService _redisService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<DbExecuteService> _logger;
        private const string RedisBufferKey = "exec-buffer";
        private static readonly TimeSpan _delay = TimeSpan.FromMilliseconds(500);

        public DbExecuteService(RedisService redisService,
            IConfiguration configuration,
            IServiceProvider serviceProvider,
            ILogger<DbExecuteService> logger)
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

        private async Task OnWork(int i, CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                var bufferKey = $"{RedisBufferKey}:{i}";
                try
                {
                    var connRedis = _redisService.Connection;
                    var success = await connRedis.Sync("write-back", async () =>
                    {
                        var result = await connRedis.ScriptEvaluateAsync("pop_sql_range.lua", new
                        {
                            key = new RedisKey(bufferKey),
                            count = 100
                        });
                        if (result.Length == 0)
                            return false;

                        await using var connection = _dbContext.Connection(i);
                        var backgroundCommitEntryList = ((RedisResult[])result).Select(x => JsonConvert.DeserializeObject<BackgroundCommitEntry>(x.ToString()));
                        foreach (var g in backgroundCommitEntryList.GroupBy(x => x.RedisKey))
                        {
                            var redisKey = g.Key;
                            var sql = string.Join(Environment.NewLine, g.Select(x => x.SQL));
                            await connection.ExecuteAsync(sql);

                            await connRedis.ScriptEvaluateAsync("end_of_ref.lua", new
                            {
                                key = new RedisKey(Db.Redis.Const.ReferenceCountKey),
                                field = new RedisValue(redisKey),
                                count = g.Count(),
                                expiry = (int)Db.Redis.Const.CacheTimeToLive.TotalSeconds,
                            });
                        }
                        return true;
                    });

                    if (!success)
                        await Task.Delay(_delay, stoppingToken);
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
