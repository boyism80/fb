using NPOI.SS.UserModel;
using NPOI.XSSF.UserModel;

namespace MapEditor.Table
{
    /// <summary>
    /// Sheet grouped by map: a row with only the first column set is the map header,
    /// the following rows with an empty first column are that map's entries.
    /// Only the edited group is rewritten; other rows, sheets and styles stay as they are.
    /// </summary>
    public class GroupedSheet
    {
        private readonly ISheet _sheet;
        private readonly int _columns;
        private readonly Dictionary<int, (int Header, int End)> _groups = new Dictionary<int, (int Header, int End)>();

        public string Name => _sheet.SheetName;
        public IEnumerable<int> MapIds => _groups.Keys;

        public GroupedSheet(XSSFWorkbook workbook, string sheetName, int columns)
        {
            _sheet = workbook.GetSheet(sheetName) ?? throw new InvalidOperationException($"sheet {sheetName} not found");
            _columns = columns;
            Index();
        }

        private void Index()
        {
            _groups.Clear();
            var current = -1;
            var header = -1;
            for (int r = XlsxFile.FirstDataRow; r <= _sheet.LastRowNum; r++)
            {
                var key = XlsxFile.Text(_sheet.GetRow(r), 0);
                if (key == "")
                    continue;

                if (current >= 0)
                    _groups[current] = (header, r);

                current = int.TryParse(key, out var id) ? id : -1;
                header = r;
            }
            if (current >= 0)
                _groups[current] = (header, _sheet.LastRowNum + 1);
        }

        public List<string[]> Read(int mapId)
        {
            var entries = new List<string[]>();
            if (_groups.TryGetValue(mapId, out var group) == false)
                return entries;

            for (int r = group.Header + 1; r < group.End; r++)
            {
                var row = _sheet.GetRow(r);
                var values = new string[_columns];
                var empty = true;
                for (int c = 1; c < _columns; c++)
                {
                    values[c] = XlsxFile.Text(row, c);
                    empty &= values[c] == "";
                }
                if (empty == false)
                    entries.Add(values);
            }
            return entries;
        }

        public void Write(int mapId, List<string[]> entries)
        {
            if (_groups.TryGetValue(mapId, out var group) == false)
            {
                if (entries.Count == 0)
                    return;

                var headerRow = _sheet.CreateRow(_sheet.LastRowNum + 1);
                headerRow.CreateCell(0).SetCellValue(mapId.ToString());
                group = (headerRow.RowNum, headerRow.RowNum + 1);
            }

            var oldCount = group.End - group.Header - 1;
            var start = group.Header + 1;
            var last = _sheet.LastRowNum;
            if (entries.Count > oldCount && group.End <= last)
            {
                _sheet.ShiftRows(group.End, last, entries.Count - oldCount);
            }
            else if (entries.Count < oldCount)
            {
                for (int r = start + entries.Count; r < group.End; r++)
                {
                    var row = _sheet.GetRow(r);
                    if (row != null)
                        _sheet.RemoveRow(row);
                }
                if (group.End <= last)
                    _sheet.ShiftRows(group.End, last, entries.Count - oldCount);
            }

            for (int i = 0; i < entries.Count; i++)
            {
                var row = _sheet.GetRow(start + i) ?? _sheet.CreateRow(start + i);
                var first = row.GetCell(0);
                if (first != null)
                    row.RemoveCell(first);

                for (int c = 1; c < _columns; c++)
                    XlsxFile.SetText(row, c, entries[i][c]);
            }
            Index();
        }

        /// <summary>
        /// Removes the map's header and entries.
        /// </summary>
        public void Remove(int mapId)
        {
            if (_groups.TryGetValue(mapId, out var group) == false)
                return;

            for (int r = group.Header; r < group.End; r++)
            {
                var row = _sheet.GetRow(r);
                if (row != null)
                    _sheet.RemoveRow(row);
            }
            var last = _sheet.LastRowNum;
            if (group.End <= last)
                _sheet.ShiftRows(group.End, last, group.Header - group.End);
            Index();
        }
    }
}
