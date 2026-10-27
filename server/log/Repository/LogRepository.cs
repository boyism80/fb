using Dapper;
using Http.Extension;
using Http.Service;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System.Text;
using System.Text.Json;

namespace Log.Repository
{
    public class LogRepository
    {
        private const int MaxInsertRows = 5000;
        private const int MaxInsertBytes = 4 * 1024 * 1024;
        private const string InsertPrefix = "INSERT INTO log (`timestamp`, `event`, `server_id`, `server_name`, `transaction_id`, `data`, `created_date`) VALUES ";
        private readonly DbContext _dbContext;
        private readonly ILogger<LogRepository> _logger;
        private readonly IConfiguration _configuration;
        private readonly uint _world;

        public LogRepository(DbContext dbContext, ILogger<LogRepository> logger, IConfiguration configuration)
        {
            _dbContext = dbContext;
            _logger = logger;
            _configuration = configuration;
            _world = (uint)_configuration.GetValue<int>("World", 0);
        }

        private readonly Random _random = new Random();

        public async Task BulkInsertAsync(IEnumerable<JsonElement> logs)
        {
            var logList = logs.ToList();
            if (logList.Count == 0)
            {
                return;
            }

            var shardSize = _dbContext.GetShardDbSize(_world);
            if (shardSize == 0)
            {
                throw new InvalidOperationException($"No MySQL shards available for world {_world}");
            }

            // Parse all log entries
            var entries = new List<LogEntry>();
            foreach (var log in logList)
            {
                try
                {
                    var entry = ParseLogEntry(log);
                    if (entry != null)
                    {
                        entries.Add(entry);
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to parse log entry, skipping");
                }
            }

            if (entries.Count == 0)
            {
                return;
            }

            // Get MySQL shard using random selection
            var shard = (uint)_random.Next((int)shardSize);

            try
            {
                await using var conn = _dbContext.GetDataConnection(_world, (int)shard);
                await conn.OpenAsync();
                await using var transaction = await conn.BeginTransactionAsync();
                var values = new List<string>();
                var prefixBytes = Encoding.UTF8.GetByteCount(InsertPrefix);
                var bytes = prefixBytes;

                foreach (var entry in entries)
                {
                    var value = BuildInsertValue(entry);
                    var valueBytes = Encoding.UTF8.GetByteCount(value);
                    if (prefixBytes + valueBytes > MaxInsertBytes)
                    {
                        throw new InvalidOperationException("A single log entry exceeds the INSERT size limit");
                    }

                    if (values.Count > 0 && (values.Count >= MaxInsertRows || bytes + 2 + valueBytes > MaxInsertBytes))
                    {
                        await conn.ExecuteAsync(InsertPrefix + string.Join(", ", values), transaction: transaction);
                        values.Clear();
                        bytes = prefixBytes;
                    }

                    values.Add(value);
                    bytes += valueBytes + (values.Count > 1 ? 2 : 0);
                }

                if (values.Count > 0)
                {
                    await conn.ExecuteAsync(InsertPrefix + string.Join(", ", values), transaction: transaction);
                }

                await transaction.CommitAsync();
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, $"Failed to bulk insert logs for world {_world} shard {shard}");
                throw;
            }
        }

        public async Task<bool> PingAsync()
        {
            var shardSize = _dbContext.GetShardDbSize(_world);
            if (shardSize == 0)
            {
                return false;
            }

            for (var shard = 0; shard < shardSize; shard++)
            {
                try
                {
                    await using var conn = _dbContext.GetDataConnection(_world, shard);
                    await conn.OpenAsync();
                    await conn.ExecuteScalarAsync<int>("SELECT 1");
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, $"Log database shard {shard} of world {_world} is unreachable");
                    return false;
                }
            }

            return true;
        }

        private static LogEntry ParseLogEntry(JsonElement log)
        {
            try
            {
                string timestampStr;
                if (log.TryGetProperty("timestamp", out var ts) && ts.ValueKind == System.Text.Json.JsonValueKind.String)
                {
                    timestampStr = ts.GetString() ?? DateTimeOffset.UtcNow.ToString("yyyy-MM-dd HH:mm:ss");
                }
                else
                {
                    timestampStr = DateTimeOffset.UtcNow.ToString("yyyy-MM-dd HH:mm:ss");
                }

                string transactionId = null;
                if (log.TryGetProperty("transaction_id", out var txn) &&
                    txn.ValueKind == JsonValueKind.String)
                {
                    transactionId = txn.GetString();
                }

                return new LogEntry
                {
                    Timestamp = timestampStr,
                    Event = log.TryGetProperty("event", out var evt) ? evt.GetString() ?? string.Empty : string.Empty,
                    ServerId = log.TryGetProperty("server_id", out var sid) ? sid.GetString() ?? string.Empty : string.Empty,
                    ServerName = log.TryGetProperty("server_name", out var sname) ? sname.GetString() ?? string.Empty : string.Empty,
                    TransactionId = transactionId,
                    Data = log.TryGetProperty("data", out var data) ? data.GetRawText() : "{}"
                };
            }
            catch
            {
                return null;
            }
        }


        private static string BuildInsertValue(LogEntry e)
        {
            return $"({e.Timestamp.Escape()}, {e.Event.Escape()}, {e.ServerId.Escape()}, {e.ServerName.Escape()}, {e.TransactionId.Escape()}, {e.Data.Escape()}, UTC_TIMESTAMP(3))";
        }

        private class LogEntry
        {
            public string Timestamp { get; set; } = string.Empty;
            public string Event { get; set; } = string.Empty;
            public string ServerId { get; set; } = string.Empty;
            public string ServerName { get; set; } = string.Empty;
            public string TransactionId { get; set; }
            public string Data { get; set; } = "{}";
        }
    }
}
