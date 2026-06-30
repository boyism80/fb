using Dapper;
using MySqlConnector;

namespace Http.Migration
{
    public sealed class MySqlMigrationLock : IAsyncDisposable
    {
        private const int LockTimeoutSeconds = 300;

        private readonly MySqlConnection _connection;
        private readonly string _lockName;
        private readonly bool _acquired;

        private MySqlMigrationLock(MySqlConnection connection, string lockName, bool acquired)
        {
            _connection = connection;
            _lockName = lockName;
            _acquired = acquired;
        }

        public static async Task<MySqlMigrationLock> AcquireAsync(
            string connectionString,
            MigrationScope scope,
            CancellationToken cancellationToken = default)
        {
            await EnsureDatabaseExistsAsync(connectionString, cancellationToken);

            var connection = new MySqlConnection(connectionString);
            await connection.OpenAsync(cancellationToken);

            var lockName = BuildLockName(connectionString, scope);
            var result = await connection.ExecuteScalarAsync<int?>(
                new CommandDefinition(
                    "SELECT GET_LOCK(@lockName, @timeoutSeconds);",
                    new { lockName, timeoutSeconds = LockTimeoutSeconds },
                    cancellationToken: cancellationToken));

            var acquired = result == 1;
            if (!acquired)
            {
                await connection.DisposeAsync();
                throw new InvalidOperationException($"Failed to acquire migration lock '{lockName}' within {LockTimeoutSeconds} seconds.");
            }

            return new MySqlMigrationLock(connection, lockName, acquired);
        }

        public async ValueTask DisposeAsync()
        {
            if (!_acquired)
            {
                return;
            }

            try
            {
                await _connection.ExecuteAsync("SELECT RELEASE_LOCK(@lockName);", new { lockName = _lockName });
            }
            finally
            {
                await _connection.DisposeAsync();
            }
        }

        public MySqlConnection Connection => _connection;

        private static string BuildLockName(string connectionString, MigrationScope scope)
        {
            var builder = new MySqlConnectionStringBuilder(connectionString);
            var fingerprintSource = $"{builder.Server}|{builder.Port}|{builder.Database}";
            var fingerprintBytes = System.Security.Cryptography.SHA256.HashData(System.Text.Encoding.UTF8.GetBytes(fingerprintSource));
            var fingerprint = Convert.ToHexString(fingerprintBytes)[..16];
            var scopeName = MigrationScopeDefinitions.ToFolderName(scope);
            return $"fb:mig:{scopeName}:{fingerprint}";
        }

        private static async Task EnsureDatabaseExistsAsync(string connectionString, CancellationToken cancellationToken)
        {
            var builder = new MySqlConnectionStringBuilder(connectionString);
            var database = builder.Database;
            if (string.IsNullOrWhiteSpace(database))
            {
                return;
            }

            builder.Database = string.Empty;
            await using var connection = new MySqlConnection(builder.ConnectionString);
            await connection.OpenAsync(cancellationToken);

            var escapedDatabase = database.Replace("`", "``");
            await connection.ExecuteAsync(
                new CommandDefinition(
                    $"CREATE DATABASE IF NOT EXISTS `{escapedDatabase}` DEFAULT CHARACTER SET euckr;",
                    cancellationToken: cancellationToken));
        }
    }
}
