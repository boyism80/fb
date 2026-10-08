using System.ComponentModel;
using System.Globalization;
using System.IO;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Asset;
using MapEditor.Edit;
using MapEditor.Format;
using ModelContextProtocol.Protocol;
using ModelContextProtocol.Server;

namespace MapEditor.Mcp
{
    public partial class MapTools
    {
        /// <summary>
        /// The corpus with the open map's live cells, plus map names; read on the UI thread.
        /// </summary>
        private async Task<(Dictionary<int, ServerMap> Maps, Dictionary<int, string> Names)> Snapshot(string name)
        {
            var corpus = await MapCorpus.Load(_editor.Settings.MapDirectory);
            return await OnUi(name, false, () =>
            {
                var maps = new Dictionary<int, ServerMap>(corpus);
                var doc = _editor.Document;
                if (doc != null)
                    maps[doc.Id] = doc.Map;
                var names = _editor.MapChoices.GroupBy(e => e.Id).ToDictionary(g => g.Key, g => g.First().Name);
                return (maps, names);
            });
        }

        private static ushort[] Layer(ServerMap map, string layer)
        {
            if (layer == "tile")
                return map.Tiles;
            else if (layer == "object")
                return map.Objects;
            else
                throw new ArgumentException("layer must be tile or object");
        }

        [McpServerTool(Name = "create_map"), Description("Create a new map: {id}.map filled with one tile, an empty {id}.block and a map.xlsx row on 'sheet' " +
                                                      "whose server settings (bgm, effect, host, option, revive, ...) are copied from the template map; then open it. " +
                                                      "Use an unused id (list_maps; ids above 53151 are free). Requires write permission.")]
        public async Task<string> CreateMap(int id, string name, int width, int height, int tile,
                                            [Description("existing map id whose map.xlsx settings are copied")] int template,
                                            [Description("map.xlsx sheet, map.0 .. map.25")] string sheet = "map.0")
        {
            var task = await OnUi("create_map", true, () =>
            {
                var assets = _editor.Assets ?? throw new InvalidOperationException("no client resources loaded");
                if (id <= 0 || id > 65535)
                    throw new ArgumentException("id must be 1..65535");
                if (string.IsNullOrWhiteSpace(name))
                    throw new ArgumentException("name is required");
                if (width < 8 || height < 8 || width > 255 || height > 255)
                    throw new ArgumentException("width and height must be 8..255");
                if (tile < 0 || tile >= assets.TileCount)
                    throw new ArgumentException($"tile must be 0..{assets.TileCount - 1}");
                return _editor.CreateMap(id, name, width, height, tile, sheet, template);
            });
            await task;
            return await GetMapInfo();
        }

        [McpServerTool(Name = "map_usage"), Description("Most used tile or object ids over all server maps, or only maps whose name contains 'query' " +
                                                     "(e.g. a town name) or whose id is in 'maps'. The first call loads every map (a few seconds).")]
        public async Task<string> MapUsage([Description("tile or object")] string layer, string query = "", int[] maps = null, int limit = 40)
        {
            var (corpus, names) = await Snapshot("map_usage");
            return await Task.Run(() =>
            {
                var chosen = corpus.Where(m => (maps == null || maps.Length == 0 || maps.Contains(m.Key)) &&
                                               (string.IsNullOrWhiteSpace(query) || (names.TryGetValue(m.Key, out var n) && n.Contains(query, StringComparison.OrdinalIgnoreCase))))
                                   .ToList();
                var cells = new Dictionary<int, long>();
                var mapCounts = new Dictionary<int, int>();
                foreach (var (_, map) in chosen)
                {
                    var seen = new HashSet<int>();
                    foreach (var value in Layer(map, layer))
                    {
                        if (layer == "object" && value == 0)
                            continue;
                        cells[value] = cells.GetValueOrDefault(value) + 1;
                        if (seen.Add(value))
                            mapCounts[value] = mapCounts.GetValueOrDefault(value) + 1;
                    }
                }
                return Json(new
                {
                    scannedMaps = chosen.Count,
                    sampleMaps = chosen.Take(20).Select(m => new { id = m.Key, name = names.GetValueOrDefault(m.Key, ""), m.Value.Width, m.Value.Height }),
                    ids = cells.OrderByDescending(c => c.Value).Take(Math.Clamp(limit, 1, 500)).Select(c => new { id = c.Key, cells = c.Value, maps = mapCounts[c.Key] }),
                });
            });
        }

        [McpServerTool(Name = "neighbors"), Description("How one tile or object id is used across all server maps: the most common ids next to it on the same layer " +
                                                     "(north, east, south, west, as {id, count}), the tiles under an object or objects on a tile, " +
                                                     "and the maps that use it most (good references to copy from).")]
        public async Task<string> Neighbors([Description("tile or object")] string layer, int id, int limit = 8)
        {
            var (corpus, names) = await Snapshot("neighbors");
            return await Task.Run(() =>
            {
                var directions = new[] { (0, -1), (1, 0), (0, 1), (-1, 0) };
                var around = directions.Select(_ => new Dictionary<int, long>()).ToArray();
                var other = new Dictionary<int, long>();
                var perMap = new Dictionary<int, long>();
                long total = 0;
                foreach (var (mapId, map) in corpus)
                {
                    var values = Layer(map, layer);
                    var others = layer == "tile" ? map.Objects : map.Tiles;
                    for (int y = 0; y < map.Height; y++)
                    {
                        for (int x = 0; x < map.Width; x++)
                        {
                            var i = y * map.Width + x;
                            if (values[i] != id)
                                continue;

                            total++;
                            perMap[mapId] = perMap.GetValueOrDefault(mapId) + 1;
                            if (layer == "object" || others[i] != 0)
                                other[others[i]] = other.GetValueOrDefault(others[i]) + 1;
                            for (int d = 0; d < 4; d++)
                            {
                                var nx = x + directions[d].Item1;
                                var ny = y + directions[d].Item2;
                                if (map.Contains(nx, ny))
                                {
                                    var value = values[ny * map.Width + nx];
                                    around[d][value] = around[d].GetValueOrDefault(value) + 1;
                                }
                            }
                        }
                    }
                }

                object Top(Dictionary<int, long> counts) => counts.OrderByDescending(c => c.Value).Take(Math.Clamp(limit, 1, 50)).Select(c => new { id = c.Key, count = c.Value });
                return Json(new
                {
                    layer,
                    id,
                    cells = total,
                    maps = perMap.Count,
                    north = Top(around[0]),
                    east = Top(around[1]),
                    south = Top(around[2]),
                    west = Top(around[3]),
                    tilesUnder = layer == "object" ? Top(other) : null,
                    objectsOn = layer == "tile" ? Top(other) : null,
                    topMaps = perMap.OrderByDescending(m => m.Value).Take(10).Select(m => new { id = m.Key, name = names.GetValueOrDefault(m.Key, ""), cells = m.Value }),
                });
            });
        }

        /// <summary>
        /// 3x5 digit glyphs, rows top to bottom, 3 bits per row.
        /// </summary>
        private static readonly int[] Digits =
        {
            0b111_101_101_101_111, 0b010_110_010_010_111, 0b111_001_111_100_111, 0b111_001_111_001_111, 0b101_101_111_001_001,
            0b111_100_111_001_111, 0b111_100_111_101_111, 0b111_001_001_001_001, 0b111_101_111_101_111, 0b111_101_111_001_111,
        };

        [McpServerTool(Name = "render_palette"), Description("PNG sheet of tiles or objects with their id under each picture. Pass ids, or from + count for a " +
                                                          "consecutive range (max 256).")]
        public Task<ImageContentBlock> RenderPalette([Description("tile or object")] string layer, int[] ids = null, int from = 0, int count = 64, int columns = 16)
        {
            return OnUi("render_palette", false, () =>
            {
                var assets = _editor.Assets ?? throw new InvalidOperationException("no client resources loaded");
                if (layer != "tile" && layer != "object")
                    throw new ArgumentException("layer must be tile or object");

                var list = ids != null && ids.Length > 0 ? ids.ToList() : Enumerable.Range(from, Math.Clamp(count, 1, 256)).ToList();
                if (list.Count > 256)
                    throw new ArgumentException("at most 256 ids");

                columns = Math.Clamp(columns, 1, 32);
                var cell = assets.CellPixels;
                var boxWidth = layer == "tile" ? cell : cell * 4;
                var boxHeight = layer == "tile" ? cell : cell * 6;
                const int label = 8;
                var rows = (list.Count + columns - 1) / columns;
                var width = columns * (boxWidth + 4);
                var height = rows * (boxHeight + label + 4);
                var pixels = new uint[width * height];
                Array.Fill(pixels, 0xFF282828u);
                var box = new uint[boxWidth * boxHeight];
                for (int i = 0; i < list.Count; i++)
                {
                    var left = i % columns * (boxWidth + 4) + 2;
                    var top = i / columns * (boxHeight + label + 4) + 2;
                    Array.Fill(box, 0xFF000000u);
                    if (layer == "tile")
                        assets.DrawTile(box, boxWidth, boxHeight, 0, 0, list[i]);
                    else
                        assets.DrawObject(box, boxWidth, boxHeight, (boxWidth - cell) / 2, boxHeight - cell, list[i]);
                    for (int y = 0; y < boxHeight; y++)
                        Array.Copy(box, y * boxWidth, pixels, (top + y) * width + left, boxWidth);

                    var text = list[i].ToString();
                    var textLeft = left + (boxWidth - text.Length * 4 + 1) / 2;
                    for (int k = 0; k < text.Length; k++)
                    {
                        var glyph = Digits[text[k] - '0'];
                        for (int gy = 0; gy < 5; gy++)
                        {
                            for (int gx = 0; gx < 3; gx++)
                            {
                                if ((glyph >> (14 - gy * 3 - gx) & 1) != 0)
                                    pixels[(top + boxHeight + 2 + gy) * width + textLeft + k * 4 + gx] = 0xFFFFFFFFu;
                            }
                        }
                    }
                }
                return Png(BitmapSource.Create(width, height, 96, 96, PixelFormats.Pbgra32, null, pixels, width * 4), 2);
            });
        }

        [McpServerTool(Name = "render_map"), Description("PNG overview of a whole map (the open map with unsaved edits, or any other map id) scaled to fit maxSide " +
                                                      "pixels; good for judging layout and picking reference maps.")]
        public Task<ImageContentBlock> RenderMap(int? id = null, int maxSide = 768)
        {
            return OnUi("render_map", false, () =>
            {
                var assets = _editor.Assets ?? throw new InvalidOperationException("no client resources loaded");
                var doc = _editor.Document;
                ServerMap map;
                if (id == null || (doc != null && doc.Id == id))
                {
                    map = (doc ?? throw new InvalidOperationException("No map is open; pass id.")).Map;
                }
                else
                {
                    var path = Path.Combine(_editor.Settings.MapDirectory, $"{id:000000}.map");
                    if (File.Exists(path) == false)
                        throw new ArgumentException($"map {id} not found");
                    map = ServerMap.Read(path);
                }

                var cell = assets.CellPixels;
                var sourceWidth = map.Width * cell;
                var sourceHeight = map.Height * cell;
                var fit = Math.Min(1.0, Math.Clamp(maxSide, 64, 2048) / (double)Math.Max(sourceWidth, sourceHeight));
                var outWidth = Math.Max(1, (int)(sourceWidth * fit));
                var outHeight = Math.Max(1, (int)(sourceHeight * fit));
                var sums = new long[outWidth * outHeight * 3];
                var counts = new int[outWidth * outHeight];

                // Drawn in strips of rows so big maps never need a full-size buffer; objects below a strip still
                // reach up into it.
                const int stripRows = 8;
                var strip = new uint[sourceWidth * stripRows * cell];
                for (int top = 0; top < map.Height; top += stripRows)
                {
                    var rows = Math.Min(stripRows, map.Height - top);
                    var stripHeight = rows * cell;
                    Array.Fill(strip, 0xFF000000u);
                    for (int y = top; y < top + rows; y++)
                    {
                        for (int x = 0; x < map.Width; x++)
                            assets.DrawTile(strip, sourceWidth, stripHeight, x * cell, (y - top) * cell, map.Tiles[y * map.Width + x]);
                    }
                    for (int y = top; y < Math.Min(map.Height, top + rows + assets.Objects.MaxHeight); y++)
                    {
                        for (int x = 0; x < map.Width; x++)
                            assets.DrawObject(strip, sourceWidth, stripHeight, x * cell, (y - top) * cell, map.Objects[y * map.Width + x]);
                    }

                    for (int py = 0; py < stripHeight; py++)
                    {
                        var oy = Math.Min(outHeight - 1, (int)((top * cell + py) * fit));
                        for (int px = 0; px < sourceWidth; px++)
                        {
                            var o = oy * outWidth + Math.Min(outWidth - 1, (int)(px * fit));
                            var c = strip[py * sourceWidth + px];
                            sums[o * 3] += (c >> 16) & 0xFF;
                            sums[o * 3 + 1] += (c >> 8) & 0xFF;
                            sums[o * 3 + 2] += c & 0xFF;
                            counts[o]++;
                        }
                    }
                }

                var pixels = new uint[outWidth * outHeight];
                for (int i = 0; i < pixels.Length; i++)
                {
                    var n = Math.Max(1, counts[i]);
                    pixels[i] = 0xFF000000u | (uint)(sums[i * 3] / n) << 16 | (uint)(sums[i * 3 + 1] / n) << 8 | (uint)(sums[i * 3 + 2] / n);
                }
                return Png(BitmapSource.Create(outWidth, outHeight, 96, 96, PixelFormats.Pbgra32, null, pixels, outWidth * 4), 1);
            });
        }
    }
}
