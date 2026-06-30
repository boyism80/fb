using Dapper;
using Http.Extension;
using Http.Service;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System.Text.Json;

namespace Log.Repository
{
    public class LogRepository
    {
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
                return;

            var shardSize = _dbContext.GetShardDbSize(_world);
            if (shardSize == 0)
            {
                _logger.LogWarning("No MySQL shards available for world {World}", _world);
                return;
            }

            // Parse all log entries
            var entries = new List<LogEntry>();
            foreach (var log in logList)
            {
                try
                {
                    var entry = ParseLogEntry(log);
                    if (entry != null)
                        entries.Add(entry);
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to parse log entry, skipping");
                }
            }

            if (entries.Count == 0)
                return;

            // Get MySQL shard using random selection
            var shard = (uint)_random.Next((int)shardSize);

            try
            {
                await using var conn = _dbContext.GetDataConnection(_world, (int)shard);
                await conn.OpenAsync();

                var sql = BuildBulkInsertQuery(entries);
                await conn.ExecuteAsync(sql);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, $"Failed to bulk insert logs for world {_world} shard {shard}");
            }
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


        private static string BuildBulkInsertQuery(List<LogEntry> entries)
        {
            var values = string.Join(", ", entries.Select(e =>
                $"({e.Timestamp.Escape()}, {e.Event.Escape()}, {e.ServerId.Escape()}, {e.ServerName.Escape()}, {e.TransactionId.Escape()}, {e.Data.Escape()})"));

            return $"INSERT INTO log (`timestamp`, `event`, `server_id`, `server_name`, `transaction_id`, `data`) VALUES {values}";
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