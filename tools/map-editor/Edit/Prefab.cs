using System.Security.Cryptography;
using System.Text;
using MapEditor.Format;

namespace MapEditor.Edit
{
    public struct PrefabCell
    {
        public int Dx;
        public int Dy;
        public ushort Tile;
        public ushort Object;
        public bool Block;
    }

    /// <summary>
    /// A cluster cut from server maps: neighbouring objects that belong together (see Links) plus the blocked empty
    /// cells touching them up to four rows above (the back of a house hidden by its roof), with the tiles and blocks
    /// underneath. Clusters with the same objects at the same offsets share a key; the cells are those of the first
    /// occurrence.
    /// </summary>
    public class Prefab
    {
        public string Key { get; init; }
        public int Width { get; init; }
        public int Height { get; init; }
        public int ObjectCells { get; init; }
        public List<PrefabCell> Cells { get; init; }
        public int Count { get; set; }
        public HashSet<int> Maps { get; } = new HashSet<int>();
        public int SourceMap { get; init; }
        public int SourceX { get; init; }
        public int SourceY { get; init; }

        /// <summary>
        /// Object pairs that belong to one piece: v east (dir 0) or south (dir 1) of u in at least half of u's or v's
        /// occurrences over the corpus. A house's roof parts pass; a tree next to a fence does not.
        /// </summary>
        public static HashSet<(int U, int Dir, int V)> Links(IEnumerable<ServerMap> corpus)
        {
            var total = new Dictionary<int, long>();
            var pairs = new Dictionary<(int, int, int), long>();
            foreach (var map in corpus)
            {
                for (int y = 0; y < map.Height; y++)
                {
                    for (int x = 0; x < map.Width; x++)
                    {
                        int u = map.Objects[y * map.Width + x];
                        if (u == 0)
                            continue;
                        total[u] = total.GetValueOrDefault(u) + 1;
                        if (x + 1 < map.Width && map.Objects[y * map.Width + x + 1] != 0)
                            pairs[(u, 0, map.Objects[y * map.Width + x + 1])] = pairs.GetValueOrDefault((u, 0, map.Objects[y * map.Width + x + 1])) + 1;
                        if (y + 1 < map.Height && map.Objects[(y + 1) * map.Width + x] != 0)
                            pairs[(u, 1, map.Objects[(y + 1) * map.Width + x])] = pairs.GetValueOrDefault((u, 1, map.Objects[(y + 1) * map.Width + x])) + 1;
                    }
                }
            }
            return pairs.Where(p => p.Value * 2 >= total[p.Key.Item1] || p.Value * 2 >= total[p.Key.Item3]).Select(p => p.Key).ToHashSet();
        }

        public static Dictionary<string, Prefab> Extract(IEnumerable<(int Id, ServerMap Map, BlockFile Blocks)> maps, HashSet<(int U, int Dir, int V)> links, int minCells, int maxCells)
        {
            var result = new Dictionary<string, Prefab>();
            foreach (var (id, map, blocks) in maps)
            {
                var seen = new bool[map.Width * map.Height];
                for (int start = 0; start < seen.Length; start++)
                {
                    if (seen[start] || map.Objects[start] == 0)
                        continue;

                    var cells = new List<(int X, int Y)>();
                    var queue = new Queue<int>();
                    queue.Enqueue(start);
                    seen[start] = true;
                    while (queue.Count > 0)
                    {
                        var i = queue.Dequeue();
                        int x = i % map.Width, y = i / map.Width;
                        int u = map.Objects[i];
                        cells.Add((x, y));
                        foreach (var (nx, ny) in new[] { (x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1) })
                        {
                            if (map.Contains(nx, ny) == false)
                                continue;
                            var n = ny * map.Width + nx;
                            int v = map.Objects[n];
                            var linked = nx > x ? links.Contains((u, 0, v)) : nx < x ? links.Contains((v, 0, u)) : ny > y ? links.Contains((u, 1, v)) : links.Contains((v, 1, u));
                            if (seen[n] == false && v != 0 && linked)
                            {
                                seen[n] = true;
                                queue.Enqueue(n);
                            }
                        }
                    }
                    if (cells.Count < minCells || cells.Count > maxCells)
                        continue;

                    var left = cells.Min(c => c.X);
                    var right = cells.Max(c => c.X);
                    var top = cells.Min(c => c.Y);
                    var bottom = cells.Max(c => c.Y);
                    if (right - left >= 40 || bottom - top >= 40)
                        continue;

                    var footprint = new HashSet<(int X, int Y)>(cells);
                    var flood = new Queue<(int X, int Y)>(cells);
                    while (flood.Count > 0)
                    {
                        var (x, y) = flood.Dequeue();
                        foreach (var (nx, ny) in new[] { (x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1) })
                        {
                            if (nx < left || nx > right || ny < top - 4 || ny > bottom || map.Contains(nx, ny) == false)
                                continue;
                            if (map.Objects[ny * map.Width + nx] == 0 && blocks.Contains(nx, ny) && footprint.Add((nx, ny)))
                                flood.Enqueue((nx, ny));
                        }
                    }

                    var minY = footprint.Min(c => c.Y);
                    var signature = new StringBuilder();
                    foreach (var (x, y) in cells.OrderBy(c => c.Y).ThenBy(c => c.X))
                        signature.Append($"{x - left},{y - minY}:{map.Objects[y * map.Width + x]};");
                    var key = Convert.ToHexString(SHA1.HashData(Encoding.ASCII.GetBytes(signature.ToString())))[..10].ToLowerInvariant();

                    if (result.TryGetValue(key, out var prefab) == false)
                    {
                        prefab = new Prefab
                        {
                            Key = key,
                            Width = right - left + 1,
                            Height = bottom - minY + 1,
                            ObjectCells = cells.Count,
                            SourceMap = id,
                            SourceX = left,
                            SourceY = minY,
                            Cells = footprint.Select(c =>
                            {
                                var i = c.Y * map.Width + c.X;
                                return new PrefabCell { Dx = c.X - left, Dy = c.Y - minY, Tile = map.Tiles[i], Object = map.Objects[i], Block = blocks.Contains(c.X, c.Y) };
                            }).ToList(),
                        };
                        result[key] = prefab;
                    }
                    prefab.Count++;
                    prefab.Maps.Add(id);
                }
            }
            return result;
        }
    }
}
