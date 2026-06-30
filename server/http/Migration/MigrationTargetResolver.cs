namespace Http.Migration
{
    public sealed class MigrationTargetResolver
    {
        private readonly IConfiguration _configuration;

        public MigrationTargetResolver(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        public IReadOnlyList<MigrationTarget> Resolve(MigrationProfile profile)
        {
            var allowedScopes = MigrationScopeDefinitions.GetScopes(profile).ToHashSet();
            var targets = new List<MigrationTarget>();

            if (allowedScopes.Contains(MigrationScope.Unified))
            {
                AddUnified(targets);
            }

            if (allowedScopes.Contains(MigrationScope.UnifiedData))
            {
                AddUnifiedData(targets);
            }

            if (allowedScopes.Contains(MigrationScope.WorldGlobal)
                || allowedScopes.Contains(MigrationScope.WorldData)
                || allowedScopes.Contains(MigrationScope.WorldLogData))
            {
                AddWorldTargets(targets, allowedScopes, profile);
            }

            return Deduplicate(targets);
        }

        private void AddUnified(List<MigrationTarget> targets)
        {
            var connectionString = _configuration.GetValue<string>("ConnectionStrings:MySql:unified");
            if (string.IsNullOrWhiteSpace(connectionString))
            {
                return;
            }

            targets.Add(new MigrationTarget(MigrationScope.Unified, connectionString, "unified"));
        }

        private void AddUnifiedData(List<MigrationTarget> targets)
        {
            var dataArray = _configuration.GetSection("ConnectionStrings:MySql:unifiedData").Get<string[]>();
            if (dataArray == null)
            {
                return;
            }

            for (var index = 0; index < dataArray.Length; index++)
            {
                if (string.IsNullOrWhiteSpace(dataArray[index]))
                {
                    continue;
                }

                targets.Add(new MigrationTarget(
                    MigrationScope.UnifiedData,
                    dataArray[index],
                    $"unified-data-{index}"));
            }
        }

        private void AddWorldTargets(
            List<MigrationTarget> targets,
            HashSet<MigrationScope> allowedScopes,
            MigrationProfile profile)
        {
            var worldsSection = _configuration.GetSection("ConnectionStrings:MySql:worlds");
            foreach (var worldSection in worldsSection.GetChildren())
            {
                if (profile == MigrationProfile.Log)
                {
                    var configuredWorld = _configuration.GetValue<int>("World", 0).ToString();
                    if (!string.Equals(worldSection.Key, configuredWorld, StringComparison.Ordinal))
                    {
                        continue;
                    }
                }

                if (allowedScopes.Contains(MigrationScope.WorldGlobal))
                {
                    var global = worldSection.GetValue<string>("global");
                    if (!string.IsNullOrWhiteSpace(global))
                    {
                        targets.Add(new MigrationTarget(
                            MigrationScope.WorldGlobal,
                            global,
                            $"world-{worldSection.Key}-global"));
                    }
                }

                if (allowedScopes.Contains(MigrationScope.WorldData)
                    || allowedScopes.Contains(MigrationScope.WorldLogData))
                {
                    var dataArray = worldSection.GetSection("data").Get<string[]>();
                    if (dataArray == null)
                    {
                        continue;
                    }

                    var scope = allowedScopes.Contains(MigrationScope.WorldLogData)
                        ? MigrationScope.WorldLogData
                        : MigrationScope.WorldData;

                    for (var index = 0; index < dataArray.Length; index++)
                    {
                        if (string.IsNullOrWhiteSpace(dataArray[index]))
                        {
                            continue;
                        }

                        targets.Add(new MigrationTarget(
                            scope,
                            dataArray[index],
                            $"world-{worldSection.Key}-{MigrationScopeDefinitions.ToFolderName(scope)}-{index}"));
                    }
                }
            }
        }

        private static IReadOnlyList<MigrationTarget> Deduplicate(IReadOnlyList<MigrationTarget> targets)
        {
            return targets
                .GroupBy(target => (target.Scope, NormalizeConnectionString(target.ConnectionString)))
                .Select(group => group.First())
                .ToList();
        }

        private static string NormalizeConnectionString(string connectionString)
        {
            var builder = new MySqlConnector.MySqlConnectionStringBuilder(connectionString);
            return $"{builder.Server}|{builder.Port}|{builder.Database}|{builder.UserID}".ToLowerInvariant();
        }
    }
}
