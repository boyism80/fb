namespace AdminTool.Services
{
    public class AdminActivityLogService
    {
        private const int MaxEntries = 500;
        private readonly List<AdminActivityEntry> _entries = new();
        private readonly object _lock = new();

        public void Add(AdminActivityEntry entry)
        {
            lock (_lock)
            {
                _entries.Insert(0, entry);
                if (_entries.Count > MaxEntries)
                    _entries.RemoveRange(MaxEntries, _entries.Count - MaxEntries);
            }
        }

        public IReadOnlyList<AdminActivityEntry> GetRecent(string category, int count = 50)
        {
            lock (_lock)
            {
                return _entries
                    .Where(e => e.Category == category)
                    .Take(count)
                    .ToList();
            }
        }
    }

    public class AdminActivityEntry
    {
        public DateTime Timestamp { get; set; } = DateTime.Now;
        public string Category { get; set; } = string.Empty;
        public string Summary { get; set; } = string.Empty;
        public string Detail { get; set; }
        public string Actor { get; set; }
        public string Target { get; set; }
        public bool Success { get; set; } = true;
    }
}
