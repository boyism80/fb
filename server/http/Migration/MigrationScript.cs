namespace Http.Migration
{
    public sealed class MigrationScript
    {
        public MigrationScript(
            string version,
            string name,
            string checksum,
            string sql,
            bool useTransaction)
        {
            Version = version;
            Name = name;
            Checksum = checksum;
            Sql = sql;
            UseTransaction = useTransaction;
        }

        public string Version { get; }

        public string Name { get; }

        public string Checksum { get; }

        public string Sql { get; }

        public bool UseTransaction { get; }

        public bool IsBaseline => string.Equals(Name, "baseline", StringComparison.OrdinalIgnoreCase);
    }
}
