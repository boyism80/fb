namespace Http.Migration
{
    public static class MigrationScopeDefinitions
    {
        public static string ToFolderName(MigrationScope scope)
        {
            return scope switch
            {
                MigrationScope.Unified => "unified",
                MigrationScope.UnifiedData => "unified-data",
                MigrationScope.WorldGlobal => "world-global",
                MigrationScope.WorldData => "world-data",
                MigrationScope.WorldLogData => "world-log-data",
                _ => throw new ArgumentOutOfRangeException(nameof(scope)),
            };
        }

        public static IReadOnlyList<MigrationScope> GetScopes(MigrationProfile profile)
        {
            return profile switch
            {
                MigrationProfile.Internal => new[]
                {
                    MigrationScope.WorldGlobal,
                    MigrationScope.WorldData,
                    MigrationScope.UnifiedData,
                },
                MigrationProfile.Marketplace => new[]
                {
                    MigrationScope.Unified,
                    MigrationScope.UnifiedData,
                },
                MigrationProfile.Log => new[]
                {
                    MigrationScope.WorldLogData,
                },
                _ => throw new ArgumentOutOfRangeException(nameof(profile)),
            };
        }

        public static IReadOnlyList<string> GetSentinels(MigrationScope scope)
        {
            return scope switch
            {
                MigrationScope.Unified => new[]
                {
                    "marketplace_listing",
                    "marketplace_purchase",
                },
                MigrationScope.UnifiedData => Array.Empty<string>(),
                MigrationScope.WorldGlobal => new[]
                {
                    "name_registry",
                    "clan_name",
                    "USP_NAME_SET",
                },
                MigrationScope.WorldData => new[]
                {
                    "user",
                    "item",
                    "mail",
                    "USP_MAIL_WRITE",
                },
                MigrationScope.WorldLogData => new[]
                {
                    "log",
                },
                _ => throw new ArgumentOutOfRangeException(nameof(scope)),
            };
        }
    }
}
