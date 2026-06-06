using Dapper;
using MySqlConnector;

namespace Http.Service;

public static class WriteBackFailureRecorder
{
    public static async Task RecordAsync(
        MySqlConnection logConnection,
        uint world,
        int dbShard,
        BackgroundCommitEntry entry,
        Exception ex,
        CancellationToken cancellationToken = default)
    {
        var errorCode = 0;
        string sqlState = null;
        var message = ex.Message;
        if (message.Length > 2048)
            message = message[..2048];

        for (var e = ex; e != null; e = e.InnerException)
        {
            if (e is MySqlException mx)
            {
                errorCode = (int)mx.ErrorCode;
                sqlState = mx.SqlState;
                message = mx.Message;
                if (message.Length > 2048)
                    message = message[..2048];
                break;
            }
        }

        const string insert = """
            INSERT INTO write_back_failure (world, db_shard, hash, redis_key, sql_text, error_code, sql_state, error_message)
            VALUES (@world, @dbShard, @hash, @redisKey, @sqlText, @errorCode, @sqlState, @errorMessage)
            """;

        await logConnection.ExecuteAsync(new CommandDefinition(
            insert,
            new
            {
                world,
                dbShard,
                hash = entry.Hash,
                redisKey = entry.RedisKey,
                sqlText = entry.SQL,
                errorCode = errorCode,
                sqlState,
                errorMessage = message,
            },
            cancellationToken: cancellationToken));
    }
}
