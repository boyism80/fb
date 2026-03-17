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
using RabbitMQ.Client;
using System.Text;
using StackExchange.Redis;

namespace WriteBack.Service
{
    public class WriteBackService : BackgroundService
    {
        private readonly RedisService _redisService;
        private readonly DbContext _dbContext;
        private readonly IConfiguration _configuration;
        private readonly ILogger<Http.Service.WriteBackService> _logger;
        private readonly uint _world;
        private static readonly TimeSpan _delay = TimeSpan.FromSeconds(5);
        private const int BulkSize = 100;
        private IConnection _rabbitMqConnection;

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

            // Connect to RabbitMQ (same broker as Internal, RabbitMQ:Internal)
            try
            {
                var rabbitSection = _configuration.GetSection("RabbitMQ:Internal");
                if (!rabbitSection.Exists())
                    throw new Exception("RabbitMQ:Internal configuration not found for write-back consumer");
                var factory = new ConnectionFactory
                {
                    HostName = rabbitSection.GetValue<string>("Host"),
                    Port = rabbitSection.GetValue<int>("Port"),
                    UserName = rabbitSection.GetValue<string>("Uid"),
                    Password = rabbitSection.GetValue<string>("Pwd")
                };
                _rabbitMqConnection = factory.CreateConnection();
                _logger.LogInformation("Write-back connected to RabbitMQ");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to connect to RabbitMQ");
                throw;
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

            try
            {
                _rabbitMqConnection?.Close();
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Error closing RabbitMQ connection");
            }

            _logger.LogInformation("All write-back workers stopped for world {World}, all messages processed", _world);
        }

        private async Task OnWork(int db, CancellationToken stoppingToken)
        {
            if (_rabbitMqConnection == null || !_rabbitMqConnection.IsOpen)
            {
                _logger.LogError("RabbitMQ connection not available for world {World} DB {Db}", _world, db);
                return;
            }

            IModel channel = null;
            string queueName = null;
            try
            {
                channel = _rabbitMqConnection.CreateModel();
                channel.ExchangeDeclare(Http.Service.WriteBackService.WriteBackExchangeName, ExchangeType.Direct, durable: true);
                queueName = Http.Service.WriteBackService.GetWriteBackQueueName(_world, db);
                channel.QueueDeclare(queue: queueName, durable: true, exclusive: false, autoDelete: false, arguments: null);
                channel.QueueBind(queueName, Http.Service.WriteBackService.WriteBackExchangeName, queueName);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to declare RabbitMQ queue for world {World} DB {Db}", _world, db);
                return;
            }

            if (queueName == null)
                return;

            var shardSize = _redisService.GetShardSize(_world);
            var continuous = true;

            using (channel)
            {
                while (continuous || !stoppingToken.IsCancellationRequested)
                {
                    try
                    {
                        var messagesToAck = new List<ulong>();
                        var entryList = new List<BackgroundCommitEntry>();

                        for (var i = 0; i < BulkSize; i++)
                        {
                            var result = channel.BasicGet(queueName, autoAck: false);
                            if (result == null)
                                break;

                            try
                            {
                                var json = Encoding.UTF8.GetString(result.Body.ToArray());
                                var entry = JsonConvert.DeserializeObject<BackgroundCommitEntry>(json);
                                if (entry != null)
                                {
                                    entryList.Add(entry);
                                    messagesToAck.Add(result.DeliveryTag);
                                }
                                else
                                {
                                    channel.BasicAck(result.DeliveryTag, false);
                                }
                            }
                            catch (Exception ex)
                            {
                                _logger.LogWarning(ex, "Failed to deserialize write-back message, discarding");
                                channel.BasicAck(result.DeliveryTag, false);
                            }
                        }

                        continuous = entryList.Count == BulkSize;

                        if (entryList.Count == 0)
                        {
                            await Task.Delay(_delay, stoppingToken);
                            continue;
                        }

                        await using var dbConn = db == -1 ? _dbContext.GetGlobalConnection(_world) : _dbContext.GetDataConnection(_world, db);
                        foreach (var g in entryList.GroupBy(x => x.Hash != null ? (int)(x.Hash % shardSize) : -1))
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

                        foreach (var tag in messagesToAck)
                        {
                            try
                            {
                                channel.BasicAck(tag, false);
                            }
                            catch (Exception ex)
                            {
                                _logger.LogWarning(ex, "Failed to ack delivery tag {Tag}", tag);
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
                        await Task.Delay(_delay, stoppingToken);
                    }
                }
            }
        }
    }
}