using System.Text.Json;
using Dapper;
using Http.Extension;
using Http.Service;
using MySqlConnector;

namespace Log.Repository
{
    /// <summary>
    /// Repository for bulk inserting log entries into MySQL with sharding support.
    /// Groups logs by shard and performs efficient bulk inserts.
    /// </summary>
    public class LogRepository
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<LogRepository> _logger;

        /// <summary>
        /// Initializes a new instance of the <see cref="LogRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="logger">The logger instance.</param>
        public LogRepository(DbContext dbContext, ILogger<LogRepository> logger)
        {
            _dbContext = dbContext;
            _logger = logger;
        }

        private readonly Random _random = new Random();

        /// <summary>
        /// Bulk inserts log entries into MySQL using random sharding for performance.
        /// Distributes logs randomly across all available database shards.
        /// </summary>
        /// <param name="logs">The collection of log entries to insert.</param>
        /// <returns>A task representing the asynchronous operation.</returns>
        public async Task BulkInsertAsync(IEnumerable<JsonElement> logs)
        {
            var logList = logs.ToList();
            if (logList.Count == 0)
                return;

            if (_dbContext.SharedDbSize == 0)
            {
                _logger.LogWarning("No MySQL shards available");
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
            var shard = (uint)_random.Next((int)_dbContext.SharedDbSize);

            try
            {
                await using var conn = _dbContext.Connection(shard);
                await conn.OpenAsync();

                var sql = BuildBulkInsertQuery(entries);
                await conn.ExecuteAsync(sql);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, $"Failed to bulk insert logs for shard {shard}");
            }
        }

        /// <summary>
        /// Parses a JSON log entry into a structured LogEntry object.
        /// </summary>
        /// <param name="log">The JSON log element to parse.</param>
        /// <returns>A LogEntry object, or null if parsing fails.</returns>
        private LogEntry? ParseLogEntry(JsonElement log)
        {
            try
            {
                return new LogEntry
                {
                    Timestamp = log.TryGetProperty("timestamp", out var ts) ? ts.GetInt64() : DateTimeOffset.UtcNow.ToUnixTimeMilliseconds(),
                    Event = log.TryGetProperty("event", out var evt) ? evt.GetString() ?? string.Empty : string.Empty,
                    ServerId = log.TryGetProperty("server_id", out var sid) ? sid.GetString() ?? string.Empty : string.Empty,
                    ServerName = log.TryGetProperty("server_name", out var sname) ? sname.GetString() ?? string.Empty : string.Empty,
                    Data = log.TryGetProperty("data", out var data) ? data.GetRawText() : "{}"
                };
            }
            catch
            {
                return null;
            }
        }


        /// <summary>
        /// Builds a bulk INSERT SQL query for log entries.
        /// </summary>
        /// <param name="entries">The log entries to insert.</param>
        /// <returns>A SQL INSERT statement string.</returns>
        private static string BuildBulkInsertQuery(List<LogEntry> entries)
        {
            var values = string.Join(", ", entries.Select(e =>
                $"({e.Timestamp}, {e.Event.Escape()}, {e.ServerId.Escape()}, {e.ServerName.Escape()}, {e.Data.Escape()})"));

            return $"INSERT INTO log (`timestamp`, `event`, `server_id`, `server_name`, `data`) VALUES {values}";
        }

        /// <summary>
        /// Represents a structured log entry for database storage.
        /// </summary>
        private class LogEntry
        {
            public long Timestamp { get; set; }
            public string Event { get; set; } = string.Empty;
            public string ServerId { get; set; } = string.Empty;
            public string ServerName { get; set; } = string.Empty;
            public string Data { get; set; } = "{}";
        }
    }
}
