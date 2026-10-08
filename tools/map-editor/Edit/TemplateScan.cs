using System.Text;
using MapEditor.Format;
using MapEditor.Table;

namespace MapEditor.Edit
{
    public class TemplateScanOptions
    {
        public int MinCells { get; set; } = 10;
        public int MaxCells { get; set; } = 300;

        /// <summary>
        /// A cluster becomes a template only when it appears at least this often over all maps.
        /// </summary>
        public int MinCount { get; set; } = 3;

        /// <summary>
        /// Buildings are compact and made of mostly different pieces; outlines of rooms and long walls are not.
        /// Share of the bounding box covered by objects, and distinct object ids per object cell.
        /// </summary>
        public double MinFill { get; set; } = 0.45;
        public double MinDistinct { get; set; } = 0.5;
    }

    /// <summary>
    /// Finds buildings on every map: neighbouring objects that belong together (Prefab.Links) form a cluster, and
    /// clusters with the same objects at the same offsets that repeat often enough become templates. Wall pieces
    /// (objects that repeat themselves along a row or column) never join a cluster, so a building standing against
    /// a wall or fence is still found on its own. Identical maps (copied dungeon floors) count once. Templates hold
    /// objects only; the ground under a building differs from map to map.
    /// </summary>
    public static class TemplateScan
    {
        private class Cluster
        {
            public int Width;
            public int Height;
            public List<(int Dx, int Dy, ushort Object)> Objects;
            public List<(int Map, int X, int Y)> Places = new List<(int, int, int)>();
        }

        /// <summary>
        /// doors: every door is compared closed, so a building counts once whatever state its doors are in.
        /// </summary>
        public static List<MapTemplate> Run(IReadOnlyDictionary<int, ServerMap> corpus, Func<int, string> mapName, TemplateScanOptions options, ISet<string> existing,
                                            DoorTable doors, IProgress<(double Done, string Text)> progress, CancellationToken token)
        {
            var maps = new Dictionary<int, ServerMap>();
            foreach (var (id, original) in corpus)
            {
                token.ThrowIfCancellationRequested();
                var map = new ServerMap(original.Width, original.Height);
                Array.Copy(original.Tiles, map.Tiles, map.Tiles.Length);
                Array.Copy(doors.Closed(original.Width, original.Height, original.Objects), map.Objects, map.Objects.Length);
                maps[id] = map;
            }

            var total = maps.Count;
            var read = 0;
            var learning = maps.Values.Select(map =>
            {
                token.ThrowIfCancellationRequested();
                if (++read % 200 == 0)
                    progress.Report((0.3 * read / total, $"오브젝트 연결 학습: {read}/{total} 맵"));
                return map;
            });
            var links = Prefab.Links(learning);

            // An object is a wall piece when, in most of its occurrences, the same id comes back within two cells
            // along the row or the column (AAAA or ABAB runs).
            var occurrences = new Dictionary<ushort, int>();
            var repeats = new Dictionary<ushort, int>();
            foreach (var map in maps.Values)
            {
                token.ThrowIfCancellationRequested();
                for (int y = 0; y < map.Height; y++)
                {
                    for (int x = 0; x < map.Width; x++)
                    {
                        var u = map.Objects[y * map.Width + x];
                        if (u == 0)
                            continue;
                        occurrences[u] = occurrences.GetValueOrDefault(u) + 1;
                        var repeated = false;
                        foreach (var (nx, ny) in new[] { (x + 1, y), (x + 2, y), (x - 1, y), (x - 2, y), (x, y + 1), (x, y + 2), (x, y - 1), (x, y - 2) })
                        {
                            if (map.Contains(nx, ny) && map.Objects[ny * map.Width + nx] == u)
                            {
                                repeated = true;
                                break;
                            }
                        }
                        if (repeated)
                            repeats[u] = repeats.GetValueOrDefault(u) + 1;
                    }
                }
            }
            var walls = occurrences.Where(o => repeats.GetValueOrDefault(o.Key) * 2 >= o.Value).Select(o => o.Key).ToHashSet();

            var layouts = new HashSet<(int, int, int, long)>();
            var clusters = new Dictionary<string, Cluster>();
            var done = 0;
            foreach (var (id, map) in maps.OrderBy(m => m.Key))
            {
                token.ThrowIfCancellationRequested();
                if (++done % 100 == 0)
                    progress.Report((0.3 + 0.6 * done / total, $"묶음 찾기: {done}/{total} 맵, 후보 {clusters.Count}개"));

                long hash = 17;
                foreach (var o in map.Objects)
                    hash = hash * 31 + o;
                if (layouts.Add((map.Width, map.Height, map.Objects.Count(o => o != 0), hash)) == false)
                    continue;

                var seen = new bool[map.Width * map.Height];
                for (int start = 0; start < seen.Length; start++)
                {
                    if (seen[start] || map.Objects[start] == 0 || walls.Contains(map.Objects[start]))
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
                            if (seen[n] == false && v != 0 && linked && walls.Contains((ushort)v) == false)
                            {
                                seen[n] = true;
                                queue.Enqueue(n);
                            }
                        }
                    }
                    var left = cells.Min(c => c.X);
                    var top = cells.Min(c => c.Y);
                    var right = cells.Max(c => c.X);
                    var bottom = cells.Max(c => c.Y);

                    // Repeated pieces inside the building's own box (the middle of a long roof) belong to it;
                    // a wall running past the box does not.
                    var grow = new Queue<(int X, int Y)>(cells);
                    while (grow.Count > 0)
                    {
                        var (x, y) = grow.Dequeue();
                        int u = map.Objects[y * map.Width + x];
                        foreach (var (nx, ny) in new[] { (x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1) })
                        {
                            if (nx < left || nx > right || ny < top || ny > bottom)
                                continue;
                            var n = ny * map.Width + nx;
                            int v = map.Objects[n];
                            var linked = nx > x ? links.Contains((u, 0, v)) : nx < x ? links.Contains((v, 0, u)) : ny > y ? links.Contains((u, 1, v)) : links.Contains((v, 1, u));
                            if (seen[n] == false && v != 0 && linked)
                            {
                                seen[n] = true;
                                cells.Add((nx, ny));
                                grow.Enqueue((nx, ny));
                            }
                        }
                    }
                    if (cells.Count < options.MinCells || cells.Count > options.MaxCells)
                        continue;
                    if (right - left >= 40 || bottom - top >= 40 || right == left || bottom == top)
                        continue;
                    if (cells.Count < options.MinFill * (right - left + 1) * (bottom - top + 1))
                        continue;
                    if (cells.Select(c => map.Objects[c.Y * map.Width + c.X]).Distinct().Count() < options.MinDistinct * cells.Count)
                        continue;

                    var objects = cells.OrderBy(c => c.Y).ThenBy(c => c.X).Select(c => (c.X - left, c.Y - top, map.Objects[c.Y * map.Width + c.X])).ToList();
                    var signature = new StringBuilder();
                    foreach (var (dx, dy, obj) in objects)
                        signature.Append(signature.Length == 0 ? "" : ";").Append($"{dx},{dy}:{obj}");
                    var key = signature.ToString();
                    if (clusters.TryGetValue(key, out var cluster) == false)
                        clusters[key] = cluster = new Cluster { Width = right - left + 1, Height = bottom - top + 1, Objects = objects };
                    cluster.Places.Add((id, left, top));
                }
            }

            // A variant that is a common building plus a few extra pieces (a lamp, a sign) is left to the common one;
            // the template matcher finds the building inside the variant anyway.
            var chosen = new List<KeyValuePair<string, Cluster>>();
            foreach (var candidate in clusters.Where(c => c.Value.Places.Count >= options.MinCount && existing.Contains(c.Key) == false)
                                              .OrderByDescending(c => c.Value.Places.Count).ThenByDescending(c => c.Value.Objects.Count))
            {
                var cells = candidate.Value.Objects.ToDictionary(o => (o.Dx, o.Dy), o => o.Object);
                var variant = chosen.Any(c => c.Value.Objects.Count >= 0.7 * candidate.Value.Objects.Count &&
                                              candidate.Value.Objects.Where(o => o.Object == c.Value.Objects[0].Object).Any(anchor =>
                                                  c.Value.Objects.All(o => cells.TryGetValue((anchor.Dx - c.Value.Objects[0].Dx + o.Dx, anchor.Dy - c.Value.Objects[0].Dy + o.Dy), out var v) && v == o.Object)));
                if (variant == false)
                    chosen.Add(candidate);
            }
            chosen = chosen.OrderByDescending(c => c.Value.Objects.Count).ThenByDescending(c => c.Value.Places.Count).ToList();
            var result = new List<MapTemplate>();
            for (int k = 0; k < chosen.Count; k++)
            {
                token.ThrowIfCancellationRequested();
                if (k % 50 == 0)
                    progress.Report((0.9 + 0.1 * k / Math.Max(1, chosen.Count), $"템플릿 만들기: {k}/{chosen.Count}"));

                var cluster = chosen[k].Value;
                var cells = cluster.Objects.Select(o => new TemplateCell { Dx = o.Dx, Dy = o.Dy, Object = o.Object }).ToList();
                var source = cluster.Places[0];
                result.Add(new MapTemplate
                {
                    Name = $"자동 {cluster.Width}×{cluster.Height} {mapName(source.Map)} ({cluster.Places.Count}곳)",
                    Auto = true,
                    Width = cluster.Width,
                    Height = cluster.Height,
                    Cells = cells,
                });
            }
            progress.Report((1, $"후보 {clusters.Count}개 중 {result.Count}개"));
            return result;
        }
    }
}
