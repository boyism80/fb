using NPOI.XSSF.UserModel;

namespace MapEditor.Table
{
    public class NameEntry
    {
        public int Id { get; init; }
        public string Name { get; init; } = "";

        /// <summary>
        /// npc/mob sheets only: xlsx look and color. The server sends look + 0x7FFF and the client draws
        /// monster record (wire look - 0x8000), so the record is look - 1.
        /// </summary>
        public int Look { get; init; }
        public int Color { get; init; }
        public string Label => $"{Id} {Name}";
    }

    /// <summary>
    /// id → name from table sheets (column 0 id, column 1 name, and for npc/mob column 2 look, column 3 color).
    /// </summary>
    public class NameTable
    {
        private readonly Dictionary<int, NameEntry> _entries = new Dictionary<int, NameEntry>();

        public List<NameEntry> Entries { get; } = new List<NameEntry>();

        public static NameTable Read(XSSFWorkbook workbook, IEnumerable<string> sheets, bool appearance = false)
        {
            var table = new NameTable();
            foreach (var name in sheets)
            {
                var sheet = workbook.GetSheet(name);
                if (sheet == null)
                    continue;

                for (int r = XlsxFile.FirstDataRow; r <= sheet.LastRowNum; r++)
                {
                    var row = sheet.GetRow(r);
                    if (int.TryParse(XlsxFile.Text(row, 0), out var id) == false || table._entries.ContainsKey(id))
                        continue;

                    var look = 0;
                    var color = 0;
                    if (appearance)
                    {
                        int.TryParse(XlsxFile.Text(row, 2), out look);
                        int.TryParse(XlsxFile.Text(row, 3), out color);
                    }
                    var entry = new NameEntry { Id = id, Name = XlsxFile.Text(row, 1), Look = look, Color = color };
                    table._entries[id] = entry;
                    table.Entries.Add(entry);
                }
            }
            table.Entries.Sort((a, b) => a.Id.CompareTo(b.Id));
            return table;
        }

        public string Find(int id)
        {
            return _entries.TryGetValue(id, out var entry) ? entry.Name : "";
        }

        public NameEntry Entry(int id)
        {
            return _entries.TryGetValue(id, out var entry) ? entry : null;
        }

        public void Add(NameEntry entry)
        {
            _entries[entry.Id] = entry;
            var index = Entries.FindIndex(e => e.Id > entry.Id);
            Entries.Insert(index < 0 ? Entries.Count : index, entry);
        }

        public void Remove(int id)
        {
            _entries.Remove(id);
            Entries.RemoveAll(e => e.Id == id);
        }
    }
}
