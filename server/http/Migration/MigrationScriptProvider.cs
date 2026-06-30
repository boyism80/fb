using Microsoft.Extensions.Options;

namespace Http.Migration
{
    public sealed class MigrationScriptProvider
    {
        private readonly DatabaseMigrationOptions _options;

        public MigrationScriptProvider(IOptions<DatabaseMigrationOptions> options)
        {
            _options = options.Value;
        }

        public IReadOnlyList<MigrationScript> GetScripts(MigrationScope scope)
        {
            var scopeDirectory = Path.Combine(ResolveMigrationsRoot(), MigrationScopeDefinitions.ToFolderName(scope));
            if (!Directory.Exists(scopeDirectory))
            {
                return Array.Empty<MigrationScript>();
            }

            var scripts = new List<MigrationScript>();
            foreach (var filePath in Directory.GetFiles(scopeDirectory, "*.sql").OrderBy(Path.GetFileName, StringComparer.Ordinal))
            {
                var fileName = Path.GetFileName(filePath);
                var sql = File.ReadAllText(filePath);
                var version = SqlScriptExecutor.ParseVersion(fileName);
                var name = SqlScriptExecutor.ParseName(fileName);
                scripts.Add(new MigrationScript(
                    version,
                    name,
                    MigrationChecksum.Compute(sql),
                    sql,
                    SqlScriptExecutor.UsesTransaction(sql)));
            }

            return scripts;
        }

        private string ResolveMigrationsRoot()
        {
            if (!string.IsNullOrWhiteSpace(_options.MigrationsPath))
            {
                return _options.MigrationsPath;
            }

            var environmentPath = Environment.GetEnvironmentVariable("FB_MIGRATIONS_PATH");
            if (!string.IsNullOrWhiteSpace(environmentPath))
            {
                return environmentPath;
            }

            return Path.Combine(AppContext.BaseDirectory, "migrations");
        }
    }
}
