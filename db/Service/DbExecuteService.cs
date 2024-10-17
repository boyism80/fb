using Dapper;
using http.Redis;
using http.Service;
using MySqlConnector;
using StackExchange.Redis;

namespace Db.Service
{
    public class DbExecuteService : BackgroundService
    {
        private readonly RedisService _redisService;
        private readonly IConfiguration _configuration;
        private const string RedisBufferKey = "exec-buffer";
        private static readonly TimeSpan _delay = TimeSpan.FromMilliseconds(500);
        private static uint SharedDbSize = 0;

        public DbExecuteService(RedisService redisService, IConfiguration configuration)
        {
            _redisService = redisService;
            _configuration = configuration;

            var section = _configuration.GetSection("ConnectionStrings:MySql");
            SharedDbSize = (uint)section.GetChildren().Where(x => x.Key != "-1").Count();
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var section = _configuration.GetSection("ConnectionStrings:MySql");
            var threads = section.GetChildren().Select(x => new Thread(() => 
            {
                var task = OnWork(x.Key, stoppingToken);
                task.Wait(stoppingToken);
            })).ToArray();

            foreach (var thread in threads)
            {
                thread.Start();
            }

            while (!stoppingToken.IsCancellationRequested && threads.Any(thread => thread.IsAlive))
            {
                await Task.Delay(_delay);
            }
        }

        private async Task OnWork(string i, CancellationToken stoppingToken)
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
                    using var connection = new MySqlConnection(_configuration.GetConnectionString($"MySql:{i}"));
                    await connection.ExecuteAsync(sql);
                }
                catch (Exception e)
                {
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
            var db = modKey % SharedDbSize;
            await Post((int)db, sql);
        }
    }
}
