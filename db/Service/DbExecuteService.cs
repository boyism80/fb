using Dapper;
using http.Redis;
using http.Service;
using StackExchange.Redis;

namespace Db.Service
{
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
                    var result = await _redisService.Connection.ScriptEvaluateAsync("pop_sql_range.lua", new
                    {
                        key = new RedisKey(bufferKey),
                        count = 100
                    });
                    if (result.Length == 0)
                    {
                        await Task.Delay(_delay, stoppingToken);
                        continue;
                    }

                    var sql = string.Join(Environment.NewLine, ((RedisResult[])result).Select(x => x.ToString()));
                    await using var connection = _dbContext.Connection(i);
                    await connection.ExecuteAsync(sql);
                }
                catch (Exception e)
                {
                    _logger.LogError(e, e.Message);
                }
            }
        }

        public async Task Post(int db, string sql)
        {
            var bufferKey = $"{RedisBufferKey}:{db}";
            await _redisService.Connection.ListRightPushAsync(bufferKey, sql);
        }

        public async Task Post(uint modKey, string sql)
        {
            var db = modKey % _dbContext.SharedDbSize;
            await Post((int)db, sql);
        }
    }
}
