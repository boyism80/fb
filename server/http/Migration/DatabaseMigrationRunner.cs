using Dapper;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using MySqlConnector;

namespace Http.Migration
{
    public sealed class DatabaseMigrationRunner
    {
        private const string EnsureHistoryTableSql = """
            CREATE TABLE IF NOT EXISTS `schema_migrations` (
              `scope` varchar(64) NOT NULL,
              `version` varchar(32) NOT NULL,
              `name` varchar(190) NOT NULL,
              `checksum` char(64) NOT NULL,
              `applied_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
              `applied_by` varchar(128) NOT NULL,
              PRIMARY KEY (`scope`, `version`)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
            """;

        private readonly MigrationScriptProvider _scriptProvider;
        private readonly MigrationTargetResolver _targetResolver;
        private readonly DatabaseMigrationOptions _options;
        private readonly ILogger<DatabaseMigrationRunner> _logger;

        public DatabaseMigrationRunner(
            MigrationScriptProvider scriptProvider,
            MigrationTargetResolver targetResolver,
            IOptions<DatabaseMigrationOptions> options,
            ILogger<DatabaseMigrationRunner> logger)
        {
            _scriptProvider = scriptProvider;
            _targetResolver = targetResolver;
            _options = options.Value;
            _logger = logger;
        }

        public async Task RunAsync(MigrationProfile profile, CancellationToken cancellationToken = default)
        {
            var targets = _targetResolver.Resolve(profile);
            if (targets.Count == 0)
            {
                _logger.LogInformation("No migration targets resolved for profile {Profile}.", profile);
                return;
            }

            foreach (var target in targets)
            {
                await MigrateTargetAsync(target, cancellationToken);
            }
        }

        private async Task MigrateTargetAsync(MigrationTarget target, CancellationToken cancellationToken)
        {
            var scripts = _scriptProvider.GetScripts(target.Scope);
            if (scripts.Count == 0)
            {
                _logger.LogInformation(
                    "No migration scripts found for scope {Scope} on target {Target}.",
                    target.Scope,
                    target.Label);
                return;
            }

            _logger.LogInformation(
                "Applying migrations for scope {Scope} on target {Target}.",
                target.Scope,
                target.Label);

            await using var migrationLock = await MySqlMigrationLock.AcquireAsync(
                target.ConnectionString,
                target.Scope,
                cancellationToken);

            var connection = migrationLock.Connection;
            await connection.ExecuteAsync(new CommandDefinition(EnsureHistoryTableSql, cancellationToken: cancellationToken));

            var scopeName = MigrationScopeDefinitions.ToFolderName(target.Scope);
            var applied = await LoadAppliedMigrationsAsync(connection, scopeName, cancellationToken);
            VerifyAppliedChecksums(scripts, applied, target);

            foreach (var script in scripts)
            {
                if (applied.ContainsKey(script.Version))
                {
                    continue;
                }

                if (script.IsBaseline)
                {
                    await HandleBaselineAsync(connection, target, script, applied, cancellationToken);
                    continue;
                }

                await ApplyMigrationAsync(connection, target, script, cancellationToken);
            }
        }

        private async Task HandleBaselineAsync(
            MySqlConnection connection,
            MigrationTarget target,
            MigrationScript baseline,
            Dictionary<string, string> applied,
            CancellationToken cancellationToken)
        {
            var sentinels = MigrationScopeDefinitions.GetSentinels(target.Scope);
            if (sentinels.Count == 0)
            {
                await ApplyMigrationAsync(connection, target, baseline, cancellationToken);
                return;
            }

            var existingSentinels = 0;
            foreach (var sentinel in sentinels)
            {
                if (await ObjectExistsAsync(connection, sentinel, cancellationToken))
                {
                    existingSentinels++;
                }
            }

            if (existingSentinels == 0)
            {
                await ApplyMigrationAsync(connection, target, baseline, cancellationToken);
            }
            else if (existingSentinels == sentinels.Count)
            {
                _logger.LogInformation(
                    "Baseline already present for scope {Scope} on target {Target}. Recording migration history only.",
                    target.Scope,
                    target.Label);
                await RecordMigrationAsync(connection, target, baseline, _options.AppliedBy, cancellationToken);
            }
            else
            {
                throw new InvalidOperationException(
                    $"Partial baseline schema detected for scope {target.Scope} on target {target.Label}. Manual repair is required.");
            }
        }

        private async Task ApplyMigrationAsync(
            MySqlConnection connection,
            MigrationTarget target,
            MigrationScript script,
            CancellationToken cancellationToken)
        {
            _logger.LogInformation(
                "Applying migration {Version}_{Name} to scope {Scope} on target {Target}.",
                script.Version,
                script.Name,
                target.Scope,
                target.Label);

            var statements = SqlScriptExecutor.SplitStatements(script.Sql);
            if (script.UseTransaction)
            {
                await using var transaction = await connection.BeginTransactionAsync(cancellationToken);
                try
                {
                    foreach (var statement in statements)
                    {
                        await connection.ExecuteAsync(
                            new CommandDefinition(statement, transaction: transaction, cancellationToken: cancellationToken));
                    }

                    await RecordMigrationAsync(connection, target, script, _options.AppliedBy, cancellationToken, transaction);
                    await transaction.CommitAsync(cancellationToken);
                }
                catch
                {
                    await transaction.RollbackAsync(cancellationToken);
                    throw;
                }
            }
            else
            {
                foreach (var statement in statements)
                {
                    await connection.ExecuteAsync(new CommandDefinition(statement, cancellationToken: cancellationToken));
                }

                await RecordMigrationAsync(connection, target, script, _options.AppliedBy, cancellationToken);
            }
        }

        private static async Task RecordMigrationAsync(
            MySqlConnection connection,
            MigrationTarget target,
            MigrationScript script,
            string appliedBy,
            CancellationToken cancellationToken,
            MySqlTransaction transaction = null)
        {
            var scopeName = MigrationScopeDefinitions.ToFolderName(target.Scope);
            await connection.ExecuteAsync(
                new CommandDefinition(
                    """
                    INSERT INTO `schema_migrations` (
                        `scope`,
                        `version`,
                        `name`,
                        `checksum`,
                        `applied_by`)
                    VALUES (
                        @scope,
                        @version,
                        @name,
                        @checksum,
                        @appliedBy);
                    """,
                    new
                    {
                        scope = scopeName,
                        version = script.Version,
                        name = script.Name,
                        checksum = script.Checksum,
                        appliedBy,
                    },
                    transaction: transaction,
                    cancellationToken: cancellationToken));
        }

        private async Task<Dictionary<string, string>> LoadAppliedMigrationsAsync(
            MySqlConnection connection,
            string scopeName,
            CancellationToken cancellationToken)
        {
            var rows = await connection.QueryAsync<(string Version, string Checksum)>(
                new CommandDefinition(
                    "SELECT `version`, `checksum` FROM `schema_migrations` WHERE `scope` = @scope;",
                    new { scope = scopeName },
                    cancellationToken: cancellationToken));

            return rows.ToDictionary(row => row.Version, row => row.Checksum, StringComparer.Ordinal);
        }

        private void VerifyAppliedChecksums(
            IReadOnlyList<MigrationScript> scripts,
            IReadOnlyDictionary<string, string> applied,
            MigrationTarget target)
        {
            foreach (var script in scripts)
            {
                if (!applied.TryGetValue(script.Version, out var checksum))
                {
                    continue;
                }

                if (!string.Equals(checksum, script.Checksum, StringComparison.OrdinalIgnoreCase))
                {
                    throw new InvalidOperationException(
                        $"Checksum mismatch for migration {script.Version} in scope {target.Scope} on target {target.Label}.");
                }
            }
        }

        private static async Task<bool> ObjectExistsAsync(
            MySqlConnection connection,
            string objectName,
            CancellationToken cancellationToken)
        {
            var tableExists = await connection.ExecuteScalarAsync<int?>(
                new CommandDefinition(
                    """
                    SELECT 1
                    FROM information_schema.tables
                    WHERE table_schema = DATABASE()
                      AND table_name = @objectName
                    LIMIT 1;
                    """,
                    new { objectName },
                    cancellationToken: cancellationToken));

            if (tableExists == 1)
            {
                return true;
            }

            var routineExists = await connection.ExecuteScalarAsync<int?>(
                new CommandDefinition(
                    """
                    SELECT 1
                    FROM information_schema.routines
                    WHERE routine_schema = DATABASE()
                      AND routine_name = @objectName
                    LIMIT 1;
                    """,
                    new { objectName },
                    cancellationToken: cancellationToken));

            return routineExists == 1;
        }
    }
}
