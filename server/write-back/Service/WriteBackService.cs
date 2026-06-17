using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Redis;
using Http.Service;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using MySqlConnector;
using Newtonsoft.Json;
using RabbitMQ.Client;
using StackExchange.Redis;
using System.Text;

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

        private static readonly string EndOfRefScript = """
            local COUNT_REFS = KEYS[1]
            local EXPIRY = tonumber(ARGV[1])
            local LENGTH = tonumber(ARGV[2])

            for i = 1, LENGTH do
                local field = ARGV[2 * i + 1]
                local count = tonumber(ARGV[2 * i + 2])
                if count == nil then
                    count = 0
                end
                local ref = redis.call('hincrby', COUNT_REFS, field, -count)
                if tonumber(ref) <= 0 then
                    redis.call('hdel', COUNT_REFS, field)
                    redis.call('expire', field, EXPIRY)
                end
            end
            """;

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
            {
                throw new Exception("World configuration is required");
            }
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

            try
            {
                var rabbitSection = _configuration.GetSection("RabbitMQ:Internal");
                if (!rabbitSection.Exists())
                {
                    throw new Exception("RabbitMQ:Internal configuration not found for write-back consumer");
                }

                var factory = new ConnectionFactory
                {
                    HostName = rabbitSection.GetValue<string>("Host"),
                    Port = rabbitSection.GetValue<int>("Port"),
                    UserName = rabbitSection.GetValue<string>("Uid"),
                    Password = rabbitSection.GetValue<string>("Pwd")
                };
                _rabbitMqConnection = await factory.CreateConnectionAsync(stoppingToken);
                _logger.LogInformation("Write-back connected to RabbitMQ");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to connect to RabbitMQ");
                throw;
            }

            var workers = new List<Task>();

            var globalSection = mysqlSection.GetSection("global");
            if (globalSection.Exists())
            {
                workers.Add(OnWorkAsync(-1, stoppingToken));
            }

            var dataArray = mysqlSection.GetSection("data").Get<string[]>();
            if (dataArray != null && dataArray.Length > 0)
            {
                for (int i = 0; i < dataArray.Length; i++)
                {
                    var shardIndex = i;
                    workers.Add(OnWorkAsync(shardIndex, stoppingToken));
                }
            }

            try
            {
                await Task.WhenAll(workers);
            }
            catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
            {
                _logger.LogInformation("Write-back workers cancelled for world {World}.", _world);
            }

            try
            {
                if (_rabbitMqConnection != null)
                {
                    await _rabbitMqConnection.CloseAsync(cancellationToken: CancellationToken.None);
                    await _rabbitMqConnection.DisposeAsync();
                }
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Error closing RabbitMQ connection");
            }

            _logger.LogInformation("All write-back workers stopped for world {World}, all messages processed", _world);
        }

        private async Task OnWorkAsync(int db, CancellationToken stoppingToken)
        {
            if (_rabbitMqConnection == null || !_rabbitMqConnection.IsOpen)
            {
                _logger.LogError("RabbitMQ connection not available for world {World} DB {Db}", _world, db);
                return;
            }

            string queueName = null;
            IChannel channel = null;
            try
            {
                channel = await _rabbitMqConnection.CreateChannelAsync(new CreateChannelOptions(false, false, null, null), stoppingToken);
                await channel.ExchangeDeclareAsync(Http.Service.WriteBackService.WriteBackExchangeName, ExchangeType.Direct, durable: true, autoDelete: false, arguments: null, passive: false, noWait: false, stoppingToken);
                queueName = Http.Service.WriteBackService.GetWriteBackQueueName(_world, db);
                await channel.QueueDeclareAsync(queue: queueName, durable: true, exclusive: false, autoDelete: false, arguments: null, passive: false, noWait: false, cancellationToken: stoppingToken);
                await channel.QueueBindAsync(queueName, Http.Service.WriteBackService.WriteBackExchangeName, queueName, arguments: null, noWait: false, stoppingToken);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to declare RabbitMQ queue for world {World} DB {Db}", _world, db);
                if (channel != null)
                {
                    try
                    {
                        await channel.CloseAsync(cancellationToken: CancellationToken.None);
                    }
                    catch
                    {
                        // Ignore
                    }

                    await channel.DisposeAsync();
                }

                return;
            }

            if (queueName == null)
            {
                return;
            }

            var shardSize = _redisService.GetShardSize(_world);
            var continuous = true;

            try
            {
                while (continuous || !stoppingToken.IsCancellationRequested)
                {
                    try
                    {
                        var messagesToAck = new List<ulong>();
                        var entryList = new List<BackgroundCommitEntry>();

                        for (var i = 0; i < BulkSize; i++)
                        {
                            var result = await channel.BasicGetAsync(queueName, autoAck: false, stoppingToken);
                            if (result == null)
                            {
                                break;
                            }

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
                                    await channel.BasicAckAsync(result.DeliveryTag, false, stoppingToken);
                                }
                            }
                            catch (Exception ex)
                            {
                                _logger.LogWarning(ex, "Failed to deserialize write-back message, discarding");
                                await channel.BasicAckAsync(result.DeliveryTag, false, stoppingToken);
                            }
                        }

                        continuous = entryList.Count == BulkSize;

                        if (entryList.Count == 0)
                        {
                            await Task.Delay(_delay, stoppingToken);
                            continue;
                        }

                        if (db == -1)
                        {
                            await using var conn = _dbContext.GetGlobalConnection(_world);
                            for (var i = 0; i < entryList.Count; i++)
                            {
                                await ProcessWriteBackEntryAsync(
                                    channel,
                                    conn,
                                    conn,
                                    db,
                                    shardSize,
                                    entryList[i],
                                    messagesToAck[i],
                                    stoppingToken);
                            }
                        }
                        else
                        {
                            await using var logConn = _dbContext.GetGlobalConnection(_world);
                            await using var execConn = _dbContext.GetDataConnection(_world, db);
                            for (var i = 0; i < entryList.Count; i++)
                            {
                                await ProcessWriteBackEntryAsync(
                                    channel,
                                    execConn,
                                    logConn,
                                    db,
                                    shardSize,
                                    entryList[i],
                                    messagesToAck[i],
                                    stoppingToken);
                            }
                        }
                    }
                    catch (LogicException e)
                    {
                        if (e.Error == ErrorCode.DistributedLockFailed)
                        {
                            await Task.Delay(_delay, stoppingToken);
                        }
                        else
                        {
                            _logger.LogError(e, e.Message);
                        }
                    }
                    catch (Exception e)
                    {
                        _logger.LogError(e, e.Message);
                        await Task.Delay(_delay, stoppingToken);
                    }
                }
            }
            finally
            {
                if (channel != null)
                {
                    try
                    {
                        await channel.CloseAsync(cancellationToken: CancellationToken.None);
                    }
                    catch
                    {
                        // Ignore
                    }

                    await channel.DisposeAsync();
                }
            }
        }

        private async Task ProcessWriteBackEntryAsync(
            IChannel channel,
            MySqlConnection execConn,
            MySqlConnection logConn,
            int db,
            int shardSize,
            BackgroundCommitEntry entry,
            ulong deliveryTag,
            CancellationToken stoppingToken)
        {
            try
            {
                await execConn.ExecuteAsync(entry.SQL);
                _logger.LogInformation(entry.SQL);

                var mod = entry.Hash != null ? (int)(entry.Hash % shardSize) : -1;
                var values = new List<RedisValue>
                {
                    (int)Http.Redis.Const.CacheTimeToLive.TotalSeconds,
                    1,
                    entry.RedisKey,
                    1,
                };

                var redisRefConn = mod == -1 ? _redisService.GetGlobalConnection(_world) : _redisService.GetDataConnection(_world, mod);
                if (redisRefConn != null)
                {
                    await redisRefConn.EvalAsync(
                        EndOfRefScript,
                        [new RedisKey(Const.ReferenceCountKey)],
                        [.. values]);
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Write-back entry failed for world {World} db {Db}", _world, db);
                try
                {
                    await WriteBackFailureRecorder.RecordAsync(logConn, _world, db, entry, ex, stoppingToken);
                }
                catch (Exception logEx)
                {
                    _logger.LogCritical(logEx,
                        "Failed to insert write_back_failure for world {World} db {Db}. Message is still removed from MQ.",
                        _world,
                        db);
                }
            }
            finally
            {
                try
                {
                    await channel.BasicAckAsync(deliveryTag, false, stoppingToken);
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to ack delivery tag {Tag}", deliveryTag);
                }
            }
        }
    }
}
