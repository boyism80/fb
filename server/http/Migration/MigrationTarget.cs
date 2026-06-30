namespace Http.Migration
{
    public sealed class MigrationTarget
    {
        public MigrationTarget(MigrationScope scope, string connectionString, string label)
        {
            Scope = scope;
            ConnectionString = connectionString;
            Label = label;
        }

        public MigrationScope Scope { get; }

        public string ConnectionString { get; }

        public string Label { get; }
    }
}
