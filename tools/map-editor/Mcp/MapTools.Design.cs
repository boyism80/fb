using System.Collections.Concurrent;
using System.ComponentModel;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Edit;
using MapEditor.Format;
using ModelContextProtocol.Protocol;
using ModelContextProtocol.Server;

namespace MapEditor.Mcp
{
    public class ShapeInput
    {
        [Description("rect, ellipse or path")]
        public string Shape { get; set; }

        public int X { get; set; }
        public int Y { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }

        [Description("ellipse center and radii")]
        public double Cx { get; set; }
        public double Cy { get; set; }
        public double Rx { get; set; }
        public double Ry { get; set; }

        [Description("path: polyline points [[x, y], ...] and stroke width in cells")]
        public int[][] Points { get; set; }
        public double Size { get; set; } = 3;

        [Description("0..1 wobble of ellipse and path borders; 0 = exact")]
        public double Noise { get; set; }

        [Description("true removes the area (back to tile A) instead of painting B")]
        public bool Erase { get; set; }
    }

    public partial class MapTools
    {
        private static readonly ConcurrentDictionary<(int, int), TerrainSet> _terrainSets = new ConcurrentDictionary<(int, int), TerrainSet>();
        private static readonly ConcurrentDictionary<string, Prefab> _prefabs = new ConcurrentDictionary<string, Prefab>();
        private static HashSet<(int U, int Dir, int V)> _links;

        private async Task<TerrainSet> LearnTerrain(int a, int b)
        {
            if (_terrainSets.TryGetValue((a, b), out var cached))
                return cached;

            var corpus = await MapCorpus.Load(_editor.Settings.MapDirectory);
            var set = await Task.Run(() => TerrainSet.Learn(corpus.Values, a, b));
            _terrainSets[(a, b)] = set;
            return set;
        }

        [McpServerTool(Name = "terrain_pairs"), Description("Fill tiles (large self-adjacent areas such as grass, dirt, water) that meet the given fill tile " +
                                                         "through transition tiles on the server maps, most common first. Use the pair with terrain_set / paint_terrain.")]
        public async Task<string> TerrainPairs(int tile, int limit = 10)
        {
            var corpus = await MapCorpus.Load(_editor.Settings.MapDirectory);
            return await Task.Run(() =>
            {
                var count = new Dictionary<int, long>();
                var self = new Dictionary<int, long>();
                foreach (var map in corpus.Values)
                {
                    for (int y = 0; y < map.Height; y++)
                    {
                        for (int x = 0; x < map.Width; x++)
                        {
                            int t = map.Tiles[y * map.Width + x];
                            count[t] = count.GetValueOrDefault(t) + 1;
                            if (x + 1 < map.Width && map.Tiles[y * map.Width + x + 1] == t)
                                self[t] = self.GetValueOrDefault(t) + 1;
                            if (y + 1 < map.Height && map.Tiles[(y + 1) * map.Width + x] == t)
                                self[t] = self.GetValueOrDefault(t) + 1;
                        }
                    }
                }
                var fill = count.Where(c => c.Value >= 300 && self.GetValueOrDefault(c.Key) >= c.Value).Select(c => c.Key).ToHashSet();
                if (fill.Contains(tile) == false)
                    throw new ArgumentException($"tile {tile} is not a fill tile (rarely forms areas of itself)");

                var pairs = new Dictionary<int, long>();
                foreach (var map in corpus.Values)
                {
                    for (int y = 0; y < map.Height; y++)
                    {
                        for (int x = 0; x < map.Width; x++)
                        {
                            if (map.Tiles[y * map.Width + x] != tile)
                                continue;
                            foreach (var (dx, dy) in new[] { (1, 0), (-1, 0), (0, 1), (0, -1) })
                            {
                                for (int step = 1; step <= 3; step++)
                                {
                                    int nx = x + dx * step, ny = y + dy * step;
                                    if (map.Contains(nx, ny) == false)
                                        break;
                                    int n = map.Tiles[ny * map.Width + nx];
                                    if (fill.Contains(n))
                                    {
                                        if (n != tile && step > 1)
                                            pairs[n] = pairs.GetValueOrDefault(n) + 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                return Json(pairs.OrderByDescending(p => p.Value).Take(Math.Clamp(limit, 1, 50)).Select(p => new { tile = p.Key, count = p.Value, cells = count[p.Key] }));
            });
        }

        [McpServerTool(Name = "terrain_set"), Description("Learn the transition tiles between fill tiles a and b from the server maps (corner model). " +
                                                       "Returns, per corner pattern (which corners are b), the tiles with corpus count and confidence 0..1; " +
                                                       "tiles under 0.6 are not used by paint_terrain. Check them with render_palette.")]
        public async Task<string> TerrainSetInfo(int a, int b)
        {
            var set = await LearnTerrain(a, b);
            var names = new[] { "NW", "NE", "SW", "SE" };
            return Json(Enumerable.Range(0, 16).Select(p => new
            {
                pattern = p,
                bCorners = string.Join("+", Enumerable.Range(0, 4).Where(c => (p & (1 << c)) != 0).Select(c => names[c])),
                tiles = set.Patterns[p].Take(8).Select(e => new { tile = e.Tile, count = e.Count, confidence = e.Confidence }),
            }));
        }

        [McpServerTool(Name = "paint_terrain"), Description("Paint fill tile b over shapes on a base of fill tile a with learned transitions, in one undo step. " +
                                                         "Shapes apply in order (erase subtracts). Existing b areas and their edges merge with the new ones; " +
                                                         "cells next to the shapes are re-tiled. Edges are drawn inside the painted area, so give paths a size of 2 or more. " +
                                                         "Requires write permission.")]
        public async Task<string> PaintTerrain(int a, int b, ShapeInput[] shapes, int seed = 1, bool clearObjects = false,
                                               [Description("true blocks painted cells and unblocks erased ones (water)")] bool block = false)
        {
            var set = await LearnTerrain(a, b);
            return await OnUi("paint_terrain", true, () =>
            {
                var doc = RequireDocument();
                var map = doc.Map;
                var paint = new bool?[map.Width * map.Height];
                var random = new Random(seed);
                foreach (var shape in shapes)
                {
                    var phases = Enumerable.Range(0, 3).Select(_ => random.NextDouble() * Math.PI * 2).ToArray();
                    double Wobble(double t) => shape.Noise * (Math.Sin(t + phases[0]) + Math.Sin(t * 2.3 + phases[1]) / 2 + Math.Sin(t * 4.7 + phases[2]) / 3) / 1.83;

                    for (int y = 0; y < map.Height; y++)
                    {
                        for (int x = 0; x < map.Width; x++)
                        {
                            if (Inside(shape, x, y, Wobble))
                                paint[y * map.Width + x] = shape.Erase == false;
                        }
                    }
                }

                var assets = _editor.Assets ?? throw new InvalidOperationException("no client resources loaded");
                var tiles = set.Paint(map, paint, seed, assets.TileCount);
                var cells = tiles.Select(t =>
                {
                    var value = doc.Get(t.X, t.Y);
                    value.Tile = t.Tile;
                    return (t.X, t.Y, value);
                }).ToList();
                if (clearObjects || block)
                {
                    var changed = cells.ToDictionary(c => (c.X, c.Y), c => c.value);
                    for (int i = 0; i < paint.Length; i++)
                    {
                        if (paint[i] == null)
                            continue;
                        var key = (i % map.Width, i / map.Width);
                        var value = changed.TryGetValue(key, out var v) ? v : doc.Get(key.Item1, key.Item2);
                        if (clearObjects && paint[i] == true)
                            value.Object = 0;
                        if (block)
                            value.Block = paint[i].Value;
                        changed[key] = value;
                    }
                    cells = changed.Select(c => (c.Key.Item1, c.Key.Item2, c.Value)).ToList();
                }
                return Json(new { changed = cells.Count == 0 ? 0 : doc.Apply(cells) });
            });
        }

        private static bool Inside(ShapeInput shape, int x, int y, Func<double, double> wobble)
        {
            if (shape.Shape == "rect")
            {
                return x >= shape.X && x < shape.X + shape.Width && y >= shape.Y && y < shape.Y + shape.Height;
            }
            else if (shape.Shape == "ellipse")
            {
                if (shape.Rx <= 0 || shape.Ry <= 0)
                    throw new ArgumentException("ellipse needs rx and ry");
                var dx = (x - shape.Cx) / shape.Rx;
                var dy = (y - shape.Cy) / shape.Ry;
                return Math.Sqrt(dx * dx + dy * dy) <= 1 + wobble(Math.Atan2(dy, dx) * 3);
            }
            else if (shape.Shape == "path")
            {
                if (shape.Points == null || shape.Points.Length < 2)
                    throw new ArgumentException("path needs two or more points");
                double along = 0;
                for (int k = 0; k + 1 < shape.Points.Length; k++)
                {
                    double ax = shape.Points[k][0], ay = shape.Points[k][1], bx = shape.Points[k + 1][0], by = shape.Points[k + 1][1];
                    double length = Math.Max(1e-6, Math.Sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
                    double t = Math.Clamp(((x - ax) * (bx - ax) + (y - ay) * (by - ay)) / (length * length), 0, 1);
                    double px = ax + (bx - ax) * t - x, py = ay + (by - ay) * t - y;
                    if (Math.Sqrt(px * px + py * py) <= shape.Size / 2 * (1 + wobble((along + t * length) / 3)))
                        return true;
                    along += length;
                }
                return false;
            }
            else
            {
                throw new ArgumentException("shape must be rect, ellipse or path");
            }
        }

        [McpServerTool(Name = "prefabs"), Description("Reusable object clusters (houses, walls, fences, trees, wells, stalls...) cut from server maps whose name contains " +
                                                   "'query' or whose id is in 'maps': neighbouring objects that appear together consistently on all maps, plus the blocked cells behind them, with tiles " +
                                                   "and blocks. Identical clusters share a key; count = occurrences. Look at them with render_prefab, place with place_prefab.")]
        public async Task<string> Prefabs(string query = "", int[] maps = null, int minCells = 6, int maxCells = 300, int limit = 30, int minCount = 1,
                                          [Description("count (most repeated first) or size (most object cells first)")] string sort = "count")
        {
            var (corpus, names) = await Snapshot("prefabs");
            var directory = _editor.Settings.MapDirectory;
            var all = await MapCorpus.Load(directory);
            return await Task.Run(() =>
            {
                lock (_prefabs)
                    _links ??= Prefab.Links(all.Values);
                var chosen = corpus.Where(m => (maps == null || maps.Length == 0 || maps.Contains(m.Key)) &&
                                               (string.IsNullOrWhiteSpace(query) || (names.TryGetValue(m.Key, out var n) && n.Contains(query, StringComparison.OrdinalIgnoreCase))))
                                   .Select(m => (m.Key, m.Value, MapCorpus.Blocks(directory, m.Key)))
                                   .ToList();
                var found = Prefab.Extract(chosen, _links, Math.Max(1, minCells), Math.Max(minCells, maxCells));
                foreach (var prefab in found.Values)
                    _prefabs[prefab.Key] = prefab;

                return Json(new
                {
                    scannedMaps = chosen.Count,
                    prefabs = found.Values.Where(p => p.Count >= minCount)
                        .OrderByDescending(p => sort == "size" ? p.ObjectCells : p.Count).ThenByDescending(p => sort == "size" ? p.Count : p.ObjectCells)
                        .Take(Math.Clamp(limit, 1, 200))
                        .Select(p => new
                        {
                            key = p.Key,
                            p.Width,
                            p.Height,
                            p.ObjectCells,
                            blocked = p.Cells.Count(c => c.Block),
                            p.Count,
                            maps = p.Maps.Count,
                            source = new { map = p.SourceMap, name = names.GetValueOrDefault(p.SourceMap, ""), x = p.SourceX, y = p.SourceY },
                        }),
                });
            });
        }

        private static Prefab FindPrefab(string key)
        {
            return _prefabs.TryGetValue(key, out var prefab) ? prefab : throw new ArgumentException($"prefab {key} not found; call prefabs first");
        }

        [McpServerTool(Name = "render_prefab"), Description("PNG sheet of prefabs side by side in the given order, each numbered from 0 underneath " +
                                                         "(tiles and objects of its cells; .block cells tinted red when blocks is true).")]
        public Task<ImageContentBlock> RenderPrefab(string[] keys, bool blocks = false, int maxWidth = 80)
        {
            return OnUi("render_prefab", false, () =>
            {
                var assets = _editor.Assets ?? throw new InvalidOperationException("no client resources loaded");
                if (keys == null || keys.Length == 0 || keys.Length > 40)
                    throw new ArgumentException("keys must have 1..40 entries");
                var prefabs = keys.Select(FindPrefab).ToList();
                var pad = Math.Min(6, assets.Objects.MaxHeight);

                // Shelf layout in cells: pieces left to right, a new row when maxWidth is exceeded; one label row below each.
                var places = new List<(int X, int Y)>();
                int cx = 0, cy = 0, rowHeight = 0, width = 0;
                foreach (var p in prefabs)
                {
                    if (cx > 0 && cx + p.Width > maxWidth)
                    {
                        cy += rowHeight + 1;
                        cx = 0;
                        rowHeight = 0;
                    }
                    places.Add((cx, cy));
                    cx += p.Width + 1;
                    width = Math.Max(width, cx);
                    rowHeight = Math.Max(rowHeight, p.Height + pad);
                }
                var map = new ServerMap(width, cy + rowHeight + 1);
                var blockFile = new BlockFile();
                for (int k = 0; k < prefabs.Count; k++)
                {
                    foreach (var c in prefabs[k].Cells)
                    {
                        var x = places[k].X + c.Dx;
                        var y = places[k].Y + pad + c.Dy;
                        map.Tiles[y * map.Width + x] = c.Tile;
                        map.Objects[y * map.Width + x] = c.Object;
                        if (blocks && c.Block)
                            blockFile.Add(x, y);
                    }
                }

                var cell = assets.CellPixels;
                var pw = map.Width * cell;
                var pixels = DrawArea(assets, map, blockFile, 0, 0, map.Width, map.Height);
                for (int k = 0; k < prefabs.Count; k++)
                {
                    var text = k.ToString();
                    var left = places[k].X * cell + 2;
                    var top = (places[k].Y + pad + prefabs[k].Height) * cell + 4;
                    for (int i = 0; i < text.Length; i++)
                    {
                        var glyph = Digits[text[i] - '0'];
                        for (int gy = 0; gy < 10; gy++)
                        {
                            for (int gx = 0; gx < 6; gx++)
                            {
                                if ((glyph >> (14 - gy / 2 * 3 - gx / 2) & 1) != 0 && top + gy < map.Height * cell)
                                    pixels[(top + gy) * pw + left + i * 8 + gx] = 0xFFFFFF00u;
                            }
                        }
                    }
                }
                return Png(BitmapSource.Create(pw, map.Height * cell, 96, 96, PixelFormats.Pbgra32, null, pixels, pw * 4), 1);
            });
        }

        [McpServerTool(Name = "place_prefab"), Description("Place a prefab with its top-left cell at (x, y) in one undo step: objects always, tiles and blocks " +
                                                        "of its cells when the flags are set. Requires write permission.")]
        public Task<string> PlacePrefab(string key, int x, int y, bool tiles = true, bool blocks = true)
        {
            return OnUi("place_prefab", true, () =>
            {
                var doc = RequireDocument();
                var prefab = FindPrefab(key);
                var cells = prefab.Cells.Where(c => doc.Map.Contains(x + c.Dx, y + c.Dy)).Select(c =>
                {
                    var value = doc.Get(x + c.Dx, y + c.Dy);
                    value.Object = c.Object;
                    if (tiles)
                        value.Tile = c.Tile;
                    if (blocks)
                        value.Block = c.Block;
                    return (x + c.Dx, y + c.Dy, value);
                }).ToList();
                return Json(new { changed = doc.Apply(cells), prefab.Width, prefab.Height });
            });
        }

        [McpServerTool(Name = "render_layout"), Description("PNG of a design sketch: one character per cell in each row, colored by legend (character -> '#RRGGBB'), " +
                                                         "with a faint grid line every 10 cells. Use it to agree on a layout before painting.")]
        public Task<ImageContentBlock> RenderLayout(string[] rows, Dictionary<string, string> legend, int cell = 8)
        {
            return OnUi("render_layout", false, () =>
            {
                if (rows == null || rows.Length == 0)
                    throw new ArgumentException("rows are required");
                cell = Math.Clamp(cell, 2, 24);
                var colors = legend.Where(l => l.Key.Length == 1).ToDictionary(l => l.Key[0], l => 0xFF000000u | Convert.ToUInt32(l.Value.TrimStart('#'), 16));
                var width = rows.Max(r => r.Length) * cell;
                var height = rows.Length * cell;
                var pixels = new uint[width * height];
                for (int py = 0; py < height; py++)
                {
                    var row = rows[py / cell];
                    for (int px = 0; px < width; px++)
                    {
                        var cx = px / cell;
                        var color = cx < row.Length && colors.TryGetValue(row[cx], out var c) ? c : 0xFF202020u;
                        if ((cx % 10 == 0 && px % cell == 0) || (py / cell % 10 == 0 && py % cell == 0))
                            color = 0xFF000000u | ((color >> 1) & 0x7F7F7F);
                        pixels[py * width + px] = color;
                    }
                }
                return Png(BitmapSource.Create(width, height, 96, 96, PixelFormats.Pbgra32, null, pixels, width * 4), 1);
            });
        }

        [McpServerTool(Name = "check_reachability"), Description("Walk from (x, y) over cells that are not blocked (4 directions) and report warps and NPCs that " +
                                                              "cannot be reached, plus the largest unreachable walkable areas.")]
        public Task<string> CheckReachability(int x, int y)
        {
            return OnUi("check_reachability", false, () =>
            {
                var doc = RequireDocument();
                if (_editor.IsBlocked(x, y))
                    throw new ArgumentException("the start cell is blocked");

                var reached = new bool[doc.Width * doc.Height];
                var queue = new Queue<(int X, int Y)>();
                queue.Enqueue((x, y));
                reached[y * doc.Width + x] = true;
                while (queue.Count > 0)
                {
                    var (cx, cy) = queue.Dequeue();
                    foreach (var (nx, ny) in new[] { (cx - 1, cy), (cx + 1, cy), (cx, cy - 1), (cx, cy + 1) })
                    {
                        if (doc.Map.Contains(nx, ny) && reached[ny * doc.Width + nx] == false && _editor.IsBlocked(nx, ny) == false)
                        {
                            reached[ny * doc.Width + nx] = true;
                            queue.Enqueue((nx, ny));
                        }
                    }
                }

                var areas = new List<(int Size, int Left, int Top, int Right, int Bottom)>();
                var seen = (bool[])reached.Clone();
                for (int i = 0; i < seen.Length; i++)
                {
                    if (seen[i] || _editor.IsBlocked(i % doc.Width, i / doc.Width))
                        continue;
                    int size = 0, left = int.MaxValue, top = int.MaxValue, right = 0, bottom = 0;
                    var flood = new Queue<int>();
                    flood.Enqueue(i);
                    seen[i] = true;
                    while (flood.Count > 0)
                    {
                        var j = flood.Dequeue();
                        int jx = j % doc.Width, jy = j / doc.Width;
                        size++;
                        left = Math.Min(left, jx);
                        top = Math.Min(top, jy);
                        right = Math.Max(right, jx);
                        bottom = Math.Max(bottom, jy);
                        foreach (var (nx, ny) in new[] { (jx - 1, jy), (jx + 1, jy), (jx, jy - 1), (jx, jy + 1) })
                        {
                            if (doc.Map.Contains(nx, ny) && seen[ny * doc.Width + nx] == false && _editor.IsBlocked(nx, ny) == false)
                            {
                                seen[ny * doc.Width + nx] = true;
                                flood.Enqueue(ny * doc.Width + nx);
                            }
                        }
                    }
                    areas.Add((size, left, top, right, bottom));
                }

                bool Reached(int cx, int cy) => doc.Map.Contains(cx, cy) && reached[cy * doc.Width + cx];
                return Json(new
                {
                    reachable = reached.Count(r => r),
                    unreachableWalkable = areas.Sum(a => a.Size),
                    unreachableWarps = doc.Warps.Where(w => Reached(w.X, w.Y) == false).Select(w => new { w.X, w.Y, w.Dest }),
                    unreachableNpcs = doc.Npcs.Where(n => new[] { (0, 0), (1, 0), (-1, 0), (0, 1), (0, -1) }.Any(d => Reached(n.X + d.Item1, n.Y + d.Item2)) == false)
                        .Select(n => new { n.X, n.Y, n.Npc }),
                    largestUnreachable = areas.OrderByDescending(a => a.Size).Take(5).Select(a => new { size = a.Size, left = a.Left, top = a.Top, right = a.Right, bottom = a.Bottom }),
                });
            });
        }
    }
}
