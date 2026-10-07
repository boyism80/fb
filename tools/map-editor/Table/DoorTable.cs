using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using NPOI.SS.UserModel;
using NPOI.XSSF.UserModel;

namespace MapEditor.Table
{
    public class DoorPair : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public int Id { get; set; }
        public int Open { get; set; }
        public int Close { get; set; }
    }

    public class DoorModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public int Id { get; set; }

        /// <summary>
        /// door_pair ids from the left cell to the right; the door is Pairs.Count cells wide.
        /// </summary>
        public List<int> Pairs { get; set; } = new List<int>();

        public string PairsText
        {
            get => string.Join(" & ", Pairs);
            set => Pairs = value.Split('&', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)
                                .Select(v => int.TryParse(v, out var id) ? id : -1)
                                .Where(id => id >= 0)
                                .ToList();
        }
    }

    /// <summary>
    /// A door found on a map: width cells starting at (X, Y) whose objects all equal the open or the close objects.
    /// </summary>
    public class MapDoor
    {
        public DoorModel Model { get; init; }
        public int X { get; init; }
        public int Y { get; init; }
        public bool Opened { get; init; }
        public int Width => Model.Pairs.Count;
        public string Label => $"문 {Model.Id} ({X}, {Y}) 폭 {Width} {(Opened ? "열림" : "닫힘")}";
    }

    /// <summary>
    /// door.xlsx: door_pair (id, open, close) and door (id, pairs "a & b & c").
    /// </summary>
    public class DoorTable
    {
        private readonly string _path;

        public ObservableCollection<DoorPair> Pairs { get; } = new ObservableCollection<DoorPair>();
        public ObservableCollection<DoorModel> Doors { get; } = new ObservableCollection<DoorModel>();

        public DoorTable(string path)
        {
            _path = path;
        }

        public static DoorTable Read(string path)
        {
            var table = new DoorTable(path);
            if (File.Exists(path) == false)
                return table;

            var workbook = XlsxFile.Open(path);
            var pairs = workbook.GetSheet("door_pair");
            for (int r = XlsxFile.FirstDataRow; r <= pairs.LastRowNum; r++)
            {
                var row = pairs.GetRow(r);
                if (int.TryParse(XlsxFile.Text(row, 0), out var id) == false)
                    continue;

                int.TryParse(XlsxFile.Text(row, 1), out var open);
                int.TryParse(XlsxFile.Text(row, 2), out var close);
                table.Pairs.Add(new DoorPair { Id = id, Open = open, Close = close });
            }

            var doors = workbook.GetSheet("door");
            for (int r = XlsxFile.FirstDataRow; r <= doors.LastRowNum; r++)
            {
                var row = doors.GetRow(r);
                if (int.TryParse(XlsxFile.Text(row, 0), out var id) == false)
                    continue;

                table.Doors.Add(new DoorModel { Id = id, PairsText = XlsxFile.Text(row, 1) });
            }
            return table;
        }

        public DoorPair FindPair(int id)
        {
            return Pairs.FirstOrDefault(p => p.Id == id);
        }

        /// <summary>
        /// Same rule as the server's map::update_door: scan each row left to right and skip the door width on a match.
        /// </summary>
        public List<MapDoor> Find(int width, int height, ushort[] objects)
        {
            var pairs = Pairs.ToDictionary(p => p.Id);
            var models = Doors.Where(d => d.Pairs.Count > 0 && d.Pairs.All(pairs.ContainsKey)).OrderBy(d => d.Id).ToList();
            var result = new List<MapDoor>();
            for (int y = 0; y < height; y++)
            {
                var x = 0;
                while (x < width)
                {
                    MapDoor found = null;
                    foreach (var model in models)
                    {
                        if (x + model.Pairs.Count > width)
                            continue;

                        var opened = true;
                        var closed = true;
                        for (int i = 0; i < model.Pairs.Count; i++)
                        {
                            var value = objects[y * width + x + i];
                            var pair = pairs[model.Pairs[i]];
                            opened &= value == pair.Open;
                            closed &= value == pair.Close;
                        }
                        if (opened || closed)
                        {
                            found = new MapDoor { Model = model, X = x, Y = y, Opened = opened };
                            break;
                        }
                    }

                    if (found != null)
                    {
                        result.Add(found);
                        x += found.Width;
                    }
                    else
                    {
                        x++;
                    }
                }
            }
            return result;
        }

        public void Save()
        {
            var workbook = XlsxFile.Open(_path);
            WriteRows(workbook.GetSheet("door_pair"), Pairs.Select(p => new[] { p.Id.ToString(), p.Open.ToString(), p.Close.ToString() }).ToList(), 3);
            WriteRows(workbook.GetSheet("door"), Doors.Select(d => new[] { d.Id.ToString(), d.PairsText }).ToList(), 2);
            XlsxFile.Save(workbook, _path);
        }

        private static void WriteRows(ISheet sheet, List<string[]> rows, int columns)
        {
            for (int r = sheet.LastRowNum; r >= XlsxFile.FirstDataRow; r--)
            {
                var row = sheet.GetRow(r);
                if (row != null)
                    sheet.RemoveRow(row);
            }
            for (int i = 0; i < rows.Count; i++)
            {
                var row = sheet.CreateRow(XlsxFile.FirstDataRow + i);
                for (int c = 0; c < columns; c++)
                    XlsxFile.SetText(row, c, rows[i][c], numeric: true);
            }
        }
    }
}
