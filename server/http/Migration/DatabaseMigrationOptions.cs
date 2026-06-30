namespace Http.Migration
{
    public class DatabaseMigrationOptions
    {
        public const string SectionName = "Database";

        public bool AutoMigration { get; set; }

        public string MigrationsPath { get; set; } = string.Empty;

        public string AppliedBy { get; set; } = Environment.MachineName;
    }
}
