using System.ComponentModel;
using System.IO;
using System.Text;
using MapEditor.Asset;
using MapEditor.Format;

namespace MapEditor.Edit
{
    /// <summary>
    /// One cell of a template. A cell may carry only a tile, only an object or both; the missing layer is left
    /// untouched when the template is placed.
    /// </summary>
    public struct TemplateCell
    {
        public int Dx;
        public int Dy;
        public ushort? Tile;
        public ushort? Object;
    }

    /// <summary>
    /// A reusable building made of tiles and objects (no blocks, no spawns), stored in the user's template file.
    /// </summary>
    public class MapTemplate : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public int Id { get; set; }
        public string Name { get; set; } = "";

        /// <summary>
        /// Registered by the automatic scan rather than by hand.
        /// </summary>
        public bool Auto { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }
        public List<TemplateCell> Cells { get; set; } = new List<TemplateCell>();

        /// <summary>
        /// Bumped on every edit so thumbnails rebuild.
        /// </summary>
        public int Revision { get; set; }

        /// <summary>
        /// Instances found on the open map; set by the editor.
        /// </summary>
        public int OpenMapCount { get; set; }

        public int ObjectCount => Cells.Count(c => c.Object is > 0);
        public int TileCount => Cells.Count(c => c.Tile != null);
        public string SizeLabel => $"{Width}×{Height}, 오브젝트 {ObjectCount}, 타일 {TileCount}";

        public MapTemplate Clone()
        {
            return new MapTemplate { Id = Id, Name = Name, Auto = Auto, Width = Width, Height = Height, Cells = Cells.ToList() };
        }

        /// <summary>
        /// The template's objects on a Width × Height grid (0 where a cell has none).
        /// </summary>
        public ushort[] ObjectGrid()
        {
            var grid = new ushort[Width * Height];
            foreach (var cell in Cells.Where(c => c.Object is > 0))
                grid[cell.Dy * Width + cell.Dx] = cell.Object.Value;
            return grid;
        }

        /// <summary>
        /// Object offsets that identify the template on a map, with its doors closed (see DoorTable.Closed);
        /// templates without objects are identified by their tiles.
        /// </summary>
        public string Signature(Table.DoorTable doors)
        {
            var grid = doors.Closed(Width, Height, ObjectGrid());
            var keyed = ObjectCount > 0
                ? Cells.Where(c => c.Object is > 0).Select(c => (c.Dx, c.Dy, Value: (int)grid[c.Dy * Width + c.Dx]))
                : Cells.Where(c => c.Tile != null).Select(c => (c.Dx, c.Dy, Value: (int)c.Tile.Value));
            var list = keyed.ToList();
            if (list.Count == 0)
                return "";

            var left = list.Min(c => c.Dx);
            var top = list.Min(c => c.Dy);
            return string.Join(";", list.OrderBy(c => c.Dy).ThenBy(c => c.Dx).Select(c => $"{c.Dx - left},{c.Dy - top}:{c.Value}"));
        }

        /// <summary>
        /// Copies cells of a map: the tile of every cell and the object of cells that have one. The template's
        /// top-left is the top-left of the cells.
        /// </summary>
        public static MapTemplate FromCells(string name, Func<int, int, (ushort Tile, ushort Object)> get, IReadOnlyCollection<(int X, int Y)> cells)
        {
            var left = cells.Min(c => c.X);
            var top = cells.Min(c => c.Y);
            return new MapTemplate
            {
                Name = name,
                Width = cells.Max(c => c.X) - left + 1,
                Height = cells.Max(c => c.Y) - top + 1,
                Cells = cells.OrderBy(c => c.Y).ThenBy(c => c.X).Select(c =>
                {
                    var (tile, obj) = get(c.X, c.Y);
                    return new TemplateCell { Dx = c.X - left, Dy = c.Y - top, Tile = tile, Object = obj == 0 ? null : obj };
                }).ToList(),
            };
        }
    }

    /// <summary>
    /// Templates of one user in a binary file (default %APPDATA%\fb-map-editor\templates.fbt).
    /// Layout: "FBMT", u16 version, i32 count, then per template: i32 id, string name, bool auto, u16 width,
    /// u16 height, i32 cell count, cells of (u16 dx, u16 dy, u8 flags (1 tile, 2 object), u16 tile, u16 object).
    /// </summary>
    public static class TemplateFile
    {
        private const ushort Version = 1;
        private static readonly byte[] Magic = Encoding.ASCII.GetBytes("FBMT");

        public static string DefaultPath => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "fb-map-editor", "templates.fbt");

        public static List<MapTemplate> Read(string path)
        {
            if (File.Exists(path) == false)
                return new List<MapTemplate>();

            using var reader = new BinaryReader(File.OpenRead(path), Encoding.UTF8);
            if (reader.ReadBytes(4).SequenceEqual(Magic) == false)
                throw new InvalidDataException($"{path} is not a template file");
            var version = reader.ReadUInt16();
            if (version > Version)
                throw new InvalidDataException($"{path} has a newer template format ({version})");

            var count = reader.ReadInt32();
            var templates = new List<MapTemplate>(count);
            for (int i = 0; i < count; i++)
            {
                var template = new MapTemplate
                {
                    Id = reader.ReadInt32(),
                    Name = reader.ReadString(),
                    Auto = reader.ReadBoolean(),
                    Width = reader.ReadUInt16(),
                    Height = reader.ReadUInt16(),
                };
                var cells = reader.ReadInt32();
                for (int k = 0; k < cells; k++)
                {
                    var dx = reader.ReadUInt16();
                    var dy = reader.ReadUInt16();
                    var flags = reader.ReadByte();
                    var tile = reader.ReadUInt16();
                    var obj = reader.ReadUInt16();
                    template.Cells.Add(new TemplateCell
                    {
                        Dx = dx,
                        Dy = dy,
                        Tile = (flags & 1) != 0 ? tile : null,
                        Object = (flags & 2) != 0 ? obj : null,
                    });
                }
                templates.Add(template);
            }
            return templates;
        }

        /// <summary>
        /// Writes through a temp file and keeps the previous file as *.bak.
        /// </summary>
        public static void Write(string path, IEnumerable<MapTemplate> templates)
        {
            Directory.CreateDirectory(Path.GetDirectoryName(path));
            var list = templates.ToList();
            var temp = path + ".tmp";
            using (var writer = new BinaryWriter(File.Create(temp), Encoding.UTF8))
            {
                writer.Write(Magic);
                writer.Write(Version);
                writer.Write(list.Count);
                foreach (var template in list)
                {
                    writer.Write(template.Id);
                    writer.Write(template.Name ?? "");
                    writer.Write(template.Auto);
                    writer.Write((ushort)template.Width);
                    writer.Write((ushort)template.Height);
                    writer.Write(template.Cells.Count);
                    foreach (var cell in template.Cells)
                    {
                        writer.Write((ushort)cell.Dx);
                        writer.Write((ushort)cell.Dy);
                        writer.Write((byte)((cell.Tile != null ? 1 : 0) | (cell.Object != null ? 2 : 0)));
                        writer.Write(cell.Tile ?? 0);
                        writer.Write(cell.Object ?? 0);
                    }
                }
            }
            if (File.Exists(path))
                File.Replace(temp, path, path + ".bak");
            else
                File.Move(temp, path);
        }
    }

    /// <summary>
    /// A template found on a map with its top-left at (X, Y).
    /// </summary>
    public class TemplateInstance
    {
        public MapTemplate Template { get; init; }
        public int X { get; init; }
        public int Y { get; init; }

        public IEnumerable<(int X, int Y)> Cells => Template.Cells.Select(c => (X + c.Dx, Y + c.Dy));
        public bool Covers(int x, int y) => x >= X && y >= Y && x < X + Template.Width && y < Y + Template.Height;
    }

    /// <summary>
    /// A template instance on any map, with the warps on or right around it as offsets from its top-left.
    /// </summary>
    public class TemplatePlace
    {
        public int Map { get; init; }
        public string MapName { get; init; } = "";
        public int X { get; init; }
        public int Y { get; init; }
        public List<(int Dx, int Dy)> Warps { get; init; } = new List<(int, int)>();

        public string Label => $"{Map:000000} {MapName} ({X}, {Y})" + (Warps.Count == 0 ? "" : "  워프 " + string.Join(" ", Warps.Select(w => $"({w.Dx}, {w.Dy})")));
    }

    /// <summary>
    /// Finds every template on a map. A template matches where all its objects sit at their offsets (tiles when it
    /// has no objects); a door object also matches the object of the door's other state, so opening or closing a
    /// door keeps the building found. Where matches overlap, the one with more cells keeps them, so a house is not
    /// also reported as the smaller pieces it contains.
    /// </summary>
    public class TemplateMatcher
    {
        private readonly Dictionary<ushort, List<(MapTemplate Template, TemplateCell Anchor, bool ByObject)>> _anchors =
            new Dictionary<ushort, List<(MapTemplate, TemplateCell, bool)>>();
        private readonly Dictionary<ushort, List<(MapTemplate Template, TemplateCell Anchor, bool ByObject)>> _tileAnchors =
            new Dictionary<ushort, List<(MapTemplate, TemplateCell, bool)>>();
        private readonly Dictionary<ushort, HashSet<ushort>> _partners;

        /// <summary>
        /// partners: DoorTable.Partners().
        /// </summary>
        public TemplateMatcher(IEnumerable<MapTemplate> templates, Dictionary<ushort, HashSet<ushort>> partners)
        {
            _partners = partners;
            foreach (var template in templates)
            {
                var byObject = template.Cells.Any(c => c.Object is > 0);
                var keys = byObject ? template.Cells.Where(c => c.Object is > 0).ToList() : template.Cells.Where(c => c.Tile != null).ToList();
                if (keys.Count < (byObject ? 1 : 2))
                    continue;

                var anchor = keys[0];
                var index = byObject ? _anchors : _tileAnchors;
                var id = byObject ? anchor.Object.Value : anchor.Tile.Value;
                var ids = byObject && partners.TryGetValue(id, out var other) ? other.Append(id) : new[] { id };
                foreach (var key in ids)
                {
                    if (index.TryGetValue(key, out var list) == false)
                        index[key] = list = new List<(MapTemplate, TemplateCell, bool)>();
                    list.Add((template, anchor, byObject));
                }
            }
        }

        public bool Empty => _anchors.Count == 0 && _tileAnchors.Count == 0;

        public List<TemplateInstance> Find(ServerMap map)
        {
            var found = new List<(TemplateInstance Instance, int Weight)>();
            for (int y = 0; y < map.Height; y++)
            {
                for (int x = 0; x < map.Width; x++)
                {
                    var i = y * map.Width + x;
                    if (map.Objects[i] != 0 && _anchors.TryGetValue(map.Objects[i], out var byObject))
                        Match(map, x, y, byObject, found);
                    if (_tileAnchors.TryGetValue(map.Tiles[i], out var byTile))
                        Match(map, x, y, byTile, found);
                }
            }

            var claimed = new HashSet<(int, int)>();
            var result = new List<TemplateInstance>();
            foreach (var (instance, _) in found.OrderByDescending(f => f.Weight).ThenBy(f => f.Instance.Y).ThenBy(f => f.Instance.X))
            {
                var keys = instance.Template.Cells.Where(c => c.Object is > 0).Select(c => (instance.X + c.Dx, instance.Y + c.Dy)).ToList();
                if (keys.Count == 0)
                    keys = instance.Cells.ToList();
                if (keys.Any(claimed.Contains))
                    continue;

                foreach (var key in keys)
                    claimed.Add(key);
                result.Add(instance);
            }
            return result;
        }

        private void Match(ServerMap map, int x, int y, List<(MapTemplate Template, TemplateCell Anchor, bool ByObject)> candidates, List<(TemplateInstance, int)> found)
        {
            foreach (var (template, anchor, byObject) in candidates)
            {
                var left = x - anchor.Dx;
                var top = y - anchor.Dy;
                if (left < 0 || top < 0 || left + template.Width > map.Width || top + template.Height > map.Height)
                    continue;

                var match = true;
                foreach (var cell in template.Cells)
                {
                    var i = (top + cell.Dy) * map.Width + left + cell.Dx;
                    var differs = byObject
                        ? cell.Object is > 0 && map.Objects[i] != cell.Object.Value &&
                          (_partners.TryGetValue(cell.Object.Value, out var other) == false || other.Contains(map.Objects[i]) == false)
                        : cell.Tile != null && map.Tiles[i] != cell.Tile.Value;
                    if (differs)
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                    found.Add((new TemplateInstance { Template = template, X = left, Y = top }, template.ObjectCount * 4 + template.TileCount));
            }
        }
    }

    public static class TemplateRender
    {
        /// <summary>
        /// Pixels of the template: tiles, then objects row by row. Above is the number of rows added on top for
        /// objects that stack higher than the template.
        /// </summary>
        public static (uint[] Pixels, int Width, int Height, int Above) Draw(ClientAssets assets, MapTemplate template)
        {
            var above = 0;
            foreach (var cell in template.Cells.Where(c => c.Object is > 0))
            {
                var frames = assets.Objects.Find(cell.Object.Value)?.Frames.Length ?? 1;
                above = Math.Max(above, frames - 1 - cell.Dy);
            }

            var cellPixels = assets.CellPixels;
            var width = Math.Max(1, template.Width) * cellPixels;
            var height = (Math.Max(1, template.Height) + above) * cellPixels;
            var pixels = new uint[width * height];
            foreach (var cell in template.Cells.Where(c => c.Tile != null))
                assets.DrawTile(pixels, width, height, cell.Dx * cellPixels, (cell.Dy + above) * cellPixels, cell.Tile.Value);
            foreach (var cell in template.Cells.Where(c => c.Object is > 0).OrderBy(c => c.Dy).ThenBy(c => c.Dx))
                assets.DrawObject(pixels, width, height, cell.Dx * cellPixels, (cell.Dy + above) * cellPixels, cell.Object.Value);
            return (pixels, width, height, above);
        }
    }
}
