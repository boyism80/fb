using System.IO;
using System.Text.RegularExpressions;
using NPOI.XSSF.UserModel;
using PropertyChanged;

namespace MapEditor.Table
{
    public class NpcSpawn : Entity
    {
        public static readonly string[] Directions = { "TOP", "RIGHT", "BOTTOM", "LEFT" };

        public int Npc { get; set; }

        /// <summary>
        /// npc sheet row of Npc; null when the id is not in npc.xlsx.
        /// </summary>
        public NameEntry Info { get; set; }
        public string Name => Info?.Name ?? "";
        public int X { get; set; }
        public int Y { get; set; }
        public string Direction { get; set; } = "BOTTOM";

        /// <summary>
        /// Server DIRECTION order: TOP 0, RIGHT 1, BOTTOM 2, LEFT 3.
        /// </summary>
        public int DirectionIndex => Math.Max(0, Array.IndexOf(Directions, Direction?.ToUpperInvariant()));
    }

    public class MobSpawn : Entity
    {
        public int Mob { get; set; }

        /// <summary>
        /// mob sheet row of Mob; null when the id is not in mob.xlsx.
        /// </summary>
        public NameEntry Info { get; set; }
        public string Name => Info?.Name ?? "";
        public int BeginX { get; set; }
        public int BeginY { get; set; }
        public int EndX { get; set; }
        public int EndY { get; set; }
        public int Count { get; set; } = 1;
        public string Rezen { get; set; } = "00:05:00";
        public string Condition { get; set; } = "";

        public int Left => Math.Min(BeginX, EndX);
        public int Top => Math.Min(BeginY, EndY);
        public int Right => Math.Max(BeginX, EndX);
        public int Bottom => Math.Max(BeginY, EndY);
        public bool Contains(int x, int y) => x >= Left && x <= Right && y >= Top && y <= Bottom;
    }

    public class WarpEntry : Entity
    {
        private static readonly Regex MapDest = new Regex(@"^\s*map\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)\s*$");

        public int X { get; set; }
        public int Y { get; set; }
        public string Dest { get; set; } = "";
        public string Condition { get; set; } = "";

        /// <summary>
        /// map.xlsx name of DestMap; kept in step with Dest by the editor.
        /// </summary>
        public string DestName { get; set; } = "";

        /// <summary>
        /// "id name" for map destinations, the dsl text otherwise.
        /// </summary>
        [DependsOn(nameof(Dest), nameof(DestName))]
        public string DestLabel => DestMap is int id ? $"{id} {DestName}".TrimEnd() : Dest;

        /// <summary>
        /// Parts of a "map(id, x, y)" destination; null for other forms such as world(...).
        /// Setting one rewrites Dest, so only Dest goes into the undo history.
        /// </summary>
        [DependsOn(nameof(Dest))]
        public int? DestMap
        {
            get => Part(1);
            set => SetPart(value, DestX, DestY);
        }

        [DependsOn(nameof(Dest))]
        public int? DestX
        {
            get => Part(2);
            set => SetPart(DestMap, value, DestY);
        }

        [DependsOn(nameof(Dest))]
        public int? DestY
        {
            get => Part(3);
            set => SetPart(DestMap, DestX, value);
        }

        private int? Part(int group)
        {
            var match = MapDest.Match(Dest ?? "");
            return match.Success ? int.Parse(match.Groups[group].Value) : null;
        }

        private void SetPart(int? map, int? x, int? y)
        {
            Dest = $"map({map ?? 0}, {x ?? 0}, {y ?? 0})";
        }
    }

    /// <summary>
    /// npc.xlsx, mob.xlsx and map.xlsx: names (npc, mob, map.0 ~ map.25) and the spawn sheets grouped by map id
    /// (npc_spawn, mob_spawn, warp).
    /// </summary>
    public class SpawnTable
    {
        private readonly string _directory;
        private readonly XSSFWorkbook _npcBook;
        private readonly XSSFWorkbook _mobBook;
        private readonly XSSFWorkbook _mapBook;
        private readonly GroupedSheet _npcSheet;
        private readonly GroupedSheet _mobSheet;
        private readonly GroupedSheet _warpSheet;

        public NameTable NpcNames { get; }
        public NameTable MobNames { get; }
        public NameTable MapNames { get; }

        public const string NpcFile = "npc.xlsx";
        public const string MobFile = "mob.xlsx";
        public const string MapFile = "map.xlsx";

        /// <summary>
        /// Takes the opened npc/mob/map workbooks of directory; opening them is the slow part, so the caller can do
        /// it in parallel.
        /// </summary>
        public SpawnTable(string directory, XSSFWorkbook npcBook, XSSFWorkbook mobBook, XSSFWorkbook mapBook)
        {
            _directory = directory;
            _npcBook = npcBook;
            _mobBook = mobBook;
            _mapBook = mapBook;
            _npcSheet = new GroupedSheet(_npcBook, "npc_spawn", 4);
            _mobSheet = new GroupedSheet(_mobBook, "mob_spawn", 7);
            _warpSheet = new GroupedSheet(_mapBook, "warp", 4);
            NpcNames = NameTable.Read(_npcBook, new[] { "npc" }, appearance: true);
            MobNames = NameTable.Read(_mobBook, new[] { "mob" }, appearance: true);
            MapNames = NameTable.Read(_mapBook, Enumerable.Range(0, 26).Select(i => $"map.{i}"));
        }

        private string NpcPath => Path.Combine(_directory, NpcFile);
        private string MobPath => Path.Combine(_directory, MobFile);
        private string MapPath => Path.Combine(_directory, MapFile);

        public static (int X, int Y) ParsePoint(string text)
        {
            var parts = text.Split(',');
            if (parts.Length != 2 || int.TryParse(parts[0].Trim(), out var x) == false || int.TryParse(parts[1].Trim(), out var y) == false)
                return (0, 0);

            return (x, y);
        }

        private static int ParseInt(string text)
        {
            return int.TryParse(text, out var value) ? value : 0;
        }

        public List<NpcSpawn> ReadNpc(int mapId)
        {
            var result = new List<NpcSpawn>();
            foreach (var row in _npcSheet.Read(mapId))
            {
                var position = ParsePoint(row[2]);
                var npc = ParseInt(row[1]);
                result.Add(new NpcSpawn { Npc = npc, Info = NpcNames.Entry(npc), X = position.X, Y = position.Y, Direction = row[3] });
            }
            return result;
        }

        public List<MobSpawn> ReadMob(int mapId)
        {
            var result = new List<MobSpawn>();
            foreach (var row in _mobSheet.Read(mapId))
            {
                var begin = ParsePoint(row[1]);
                var end = ParsePoint(row[2]);
                var mob = ParseInt(row[4]);
                result.Add(new MobSpawn
                {
                    BeginX = begin.X,
                    BeginY = begin.Y,
                    EndX = end.X,
                    EndY = end.Y,
                    Count = ParseInt(row[3]),
                    Mob = mob,
                    Info = MobNames.Entry(mob),
                    Rezen = row[5],
                    Condition = row[6],
                });
            }
            return result;
        }

        public List<WarpEntry> ReadWarp(int mapId)
        {
            var result = new List<WarpEntry>();
            foreach (var row in _warpSheet.Read(mapId))
            {
                var before = ParsePoint(row[1]);
                var warp = new WarpEntry { X = before.X, Y = before.Y, Dest = row[2], Condition = row[3] };
                warp.DestName = warp.DestMap is int id ? MapNames.Find(id) : "";
                result.Add(warp);
            }
            return result;
        }

        /// <summary>
        /// Warp rows of every map in the sheet, keyed by map id.
        /// </summary>
        public Dictionary<int, List<WarpEntry>> ReadAllWarps()
        {
            return _warpSheet.MapIds.ToDictionary(id => id, ReadWarp);
        }

        /// <summary>
        /// NPC spawn rows of every map in the sheet, keyed by map id.
        /// </summary>
        public Dictionary<int, List<NpcSpawn>> ReadAllNpcs()
        {
            return _npcSheet.MapIds.ToDictionary(id => id, ReadNpc);
        }

        public void SaveNpc(int mapId, IEnumerable<NpcSpawn> spawns)
        {
            var rows = spawns.Select(s => new[] { "", s.Npc.ToString(), $"{s.X}, {s.Y}", s.Direction }).ToList();
            _npcSheet.Write(mapId, rows);
            XlsxFile.Save(_npcBook, NpcPath);
        }

        public void SaveMob(int mapId, IEnumerable<MobSpawn> spawns)
        {
            var rows = spawns.Select(s => new[] { "", $"{s.BeginX}, {s.BeginY}", $"{s.EndX}, {s.EndY}", s.Count.ToString(), s.Mob.ToString(), s.Rezen, s.Condition }).ToList();
            _mobSheet.Write(mapId, rows);
            XlsxFile.Save(_mobBook, MobPath);
        }

        public void SaveWarp(int mapId, IEnumerable<WarpEntry> warps)
        {
            var rows = warps.Select(w => new[] { "", $"{w.X}, {w.Y}", w.Dest, w.Condition }).ToList();
            _warpSheet.Write(mapId, rows);
            XlsxFile.Save(_mapBook, MapPath);
        }
    }
}
