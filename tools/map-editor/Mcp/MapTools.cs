using System.ComponentModel;
using System.IO;
using System.Text.Json;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Asset;
using MapEditor.Edit;
using MapEditor.Format;
using MapEditor.Table;
using MapEditor.ViewModel;
using ModelContextProtocol;
using ModelContextProtocol.Protocol;
using ModelContextProtocol.Server;

namespace MapEditor.Mcp
{
    public class CellInput
    {
        public int X { get; set; }
        public int Y { get; set; }
        public int? Tile { get; set; }
        public int? Object { get; set; }
        public bool? Block { get; set; }
    }

    /// <summary>
    /// MCP tools. Every call runs on the UI thread and edits go through MapDocument.Apply, so the user sees them
    /// immediately and can undo them.
    /// </summary>
    [McpServerToolType]
    public partial class MapTools
    {
        private readonly MainWindowViewModel _editor;

        public MapTools(MainWindowViewModel editor)
        {
            _editor = editor;
        }

        private Task<T> OnUi<T>(string name, bool write, Func<T> action)
        {
            return Application.Current.Dispatcher.InvokeAsync(() =>
            {
                _editor.McpLog.Insert(0, $"{DateTime.Now:HH:mm:ss} {name}");
                while (_editor.McpLog.Count > 200)
                    _editor.McpLog.RemoveAt(_editor.McpLog.Count - 1);

                if (write && _editor.McpAllowWrite == false)
                    throw new McpException("Write tools are disabled. Enable 'MCP 쓰기 허용' in the editor.");

                // Only McpException messages reach the client; the editor is local so the message is safe to share.
                try
                {
                    return action();
                }
                catch (Exception e) when (e is not McpException)
                {
                    throw new McpException(e.Message, e);
                }
            }).Task;
        }

        private MapDocument RequireDocument()
        {
            return _editor.Document ?? throw new InvalidOperationException("No map is open. Call open_map first.");
        }

        private static readonly JsonSerializerOptions JsonOptions = new JsonSerializerOptions
        {
            Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
        };

        private static string Json(object value)
        {
            return JsonSerializer.Serialize(value, JsonOptions);
        }

        [McpServerTool(Name = "list_maps"), Description("List server maps (id, name). Filter by id or name substring.")]
        public Task<string> ListMaps([Description("id or name substring")] string query = "", int limit = 50)
        {
            return OnUi("list_maps", false, () => Json(_editor.Maps
                .Where(m => string.IsNullOrWhiteSpace(query) || m.Label.Contains(query, StringComparison.OrdinalIgnoreCase))
                .Take(limit)
                .Select(m => new { m.Id, m.Name })));
        }

        [McpServerTool(Name = "open_map"), Description("Open a map in a new editor tab, or switch to its tab when it is already open. " +
                                                    "Other tabs keep their unsaved changes; every other tool works on the active tab.")]
        public async Task<string> OpenMap(int id)
        {
            var task = await OnUi("open_map", false, () =>
            {
                if (_editor.CanOpen == false)
                    throw new InvalidOperationException("The editor is loading or no client version is enabled (set Client paths in appsettings.{MAPEDITOR_ENVIRONMENT}.json).");

                var entry = _editor.Maps.FirstOrDefault(m => m.Id == id) ?? throw new InvalidOperationException($"map {id} not found");
                return _editor.OpenMap(entry);
            });
            await task;
            return await GetMapInfo();
        }

        [McpServerTool(Name = "get_map_info"), Description("Current map size, dirty state, counts of blocks, doors, spawns and warps.")]
        public Task<string> GetMapInfo()
        {
            return OnUi("get_map_info", false, () =>
            {
                var doc = RequireDocument();
                return Json(new
                {
                    doc.Id,
                    doc.Name,
                    doc.Width,
                    doc.Height,
                    doc.Dirty,
                    Version = _editor.Assets?.Version.ToString(),
                    Blocks = doc.Blocks.Count,
                    Doors = doc.Doors.Count,
                    Npcs = doc.Npcs.Count,
                    Mobs = doc.Mobs.Count,
                    Warps = doc.Warps.Count,
                });
            });
        }

        [McpServerTool(Name = "get_tile"), Description("Tile id, object id, .block flag, object collision bits (S=1,N=2,W=4,E=8) and effective blocked state of one cell.")]
        public Task<string> GetTile(int x, int y)
        {
            return OnUi("get_tile", false, () =>
            {
                var doc = RequireDocument();
                if (doc.Map.Contains(x, y) == false)
                    throw new ArgumentException("out of map");

                var cell = doc.Get(x, y);
                var sobj = _editor.Assets?.Objects.Find(cell.Object);
                return Json(new
                {
                    x,
                    y,
                    tile = cell.Tile,
                    @object = cell.Object,
                    block = cell.Block,
                    objectHeight = sobj?.Frames.Length ?? 0,
                    collision = sobj?.Collision ?? 0,
                    blocked = _editor.IsBlocked(x, y),
                });
            });
        }

        [McpServerTool(Name = "get_region"), Description("Rows of 'tile:object:block' (block 0/1) for a rectangle, max 64x64.")]
        public Task<string> GetRegion(int x, int y, int width, int height)
        {
            return OnUi("get_region", false, () =>
            {
                var doc = RequireDocument();
                width = Math.Clamp(width, 1, 64);
                height = Math.Clamp(height, 1, 64);
                var rows = new List<string>();
                for (int cy = y; cy < y + height; cy++)
                {
                    var cells = new List<string>();
                    for (int cx = x; cx < x + width; cx++)
                    {
                        if (doc.Map.Contains(cx, cy) == false)
                        {
                            cells.Add("-");
                            continue;
                        }

                        var cell = doc.Get(cx, cy);
                        cells.Add($"{cell.Tile}:{cell.Object}:{(cell.Block ? 1 : 0)}");
                    }
                    rows.Add(string.Join(" ", cells));
                }
                return Json(new { x, y, width, height, rows });
            });
        }

        [McpServerTool(Name = "find_cells"), Description("Coordinates whose tile and/or object equal the given ids (max 500).")]
        public Task<string> FindCells(int? tile = null, int? @object = null)
        {
            return OnUi("find_cells", false, () =>
            {
                var doc = RequireDocument();
                var result = new List<int[]>();
                for (int y = 0; y < doc.Height && result.Count < 500; y++)
                {
                    for (int x = 0; x < doc.Width && result.Count < 500; x++)
                    {
                        var cell = doc.Get(x, y);
                        if ((tile == null || cell.Tile == tile) && (@object == null || cell.Object == @object))
                            result.Add(new[] { x, y });
                    }
                }
                return Json(result);
            });
        }

        [McpServerTool(Name = "set_cells"), Description("Change cells in one undo step. Omitted fields keep their value. Requires write permission.")]
        public Task<string> SetCells(CellInput[] cells)
        {
            return OnUi("set_cells", true, () =>
            {
                var doc = RequireDocument();
                var changed = doc.Apply(cells.Where(c => doc.Map.Contains(c.X, c.Y)).Select(c =>
                {
                    var value = doc.Get(c.X, c.Y);
                    if (c.Tile != null)
                        value.Tile = (ushort)c.Tile.Value;
                    if (c.Object != null)
                        value.Object = (ushort)c.Object.Value;
                    if (c.Block != null)
                        value.Block = c.Block.Value;
                    return (c.X, c.Y, value);
                }).ToList());
                return Json(new { changed });
            });
        }

        [McpServerTool(Name = "fill_rect"), Description("Fill a rectangle on one layer ('tile', 'object' or 'block'; block value 0/1). Requires write permission.")]
        public Task<string> FillRect(string layer, int x, int y, int width, int height, int value)
        {
            return OnUi("fill_rect", true, () =>
            {
                var doc = RequireDocument();
                var cells = MainWindowViewModel.Rect(x, y, x + width - 1, y + height - 1)
                    .Where(c => doc.Map.Contains(c.X, c.Y))
                    .Select(c =>
                    {
                        var cell = doc.Get(c.X, c.Y);
                        if (layer == "tile")
                            cell.Tile = (ushort)value;
                        else if (layer == "object")
                            cell.Object = (ushort)value;
                        else if (layer == "block")
                            cell.Block = value != 0;
                        else
                            throw new ArgumentException("layer must be tile, object or block");
                        return (c.X, c.Y, cell);
                    }).ToList();
                return Json(new { changed = doc.Apply(cells) });
            });
        }

        [McpServerTool(Name = "list_doors"), Description("Doors detected on the map with the server rule (door id, position, width, opened).")]
        public Task<string> ListDoors()
        {
            return OnUi("list_doors", false, () => Json(RequireDocument().Doors.Select(d => new { door = d.Model.Id, d.X, d.Y, d.Width, d.Opened })));
        }

        [McpServerTool(Name = "toggle_door"), Description("Swap a door between open and closed objects. (x, y) can be any cell of the door. Requires write permission.")]
        public Task<string> ToggleDoor(int x, int y)
        {
            return OnUi("toggle_door", true, () =>
            {
                var door = RequireDocument().Doors.FirstOrDefault(d => d.Y == y && x >= d.X && x < d.X + d.Width)
                           ?? throw new ArgumentException("no door at the cell");
                _editor.ToggleDoor(door);
                return Json(new { door = door.Model.Id, opened = door.Opened == false });
            });
        }

        [McpServerTool(Name = "place_door"), Description("Write a door's objects starting at (x, y) to the right. Requires write permission.")]
        public Task<string> PlaceDoor(int doorId, int x, int y, bool opened)
        {
            return OnUi("place_door", true, () =>
            {
                RequireDocument();
                var model = _editor.DoorTable.Doors.FirstOrDefault(d => d.Id == doorId) ?? throw new ArgumentException($"door {doorId} not found");
                var previousModel = _editor.SelectedDoorModel;
                var previousOpened = _editor.PlaceDoorOpened;
                _editor.SelectedDoorModel = model;
                _editor.PlaceDoorOpened = opened;
                _editor.PlaceDoor(x, y);
                _editor.SelectedDoorModel = previousModel;
                _editor.PlaceDoorOpened = previousOpened;
                return Json(new { door = doorId, x, y, opened });
            });
        }

        [McpServerTool(Name = "validate"), Description("Run map validation (blocked NPC cells, spawn areas, ids outside resources, broken door pairs).")]
        public Task<string> Validate()
        {
            return OnUi("validate", false, () => Json(_editor.Validate().Select(v => new { v.X, v.Y, v.Message })));
        }

        [McpServerTool(Name = "undo"), Description("Undo the last edit. Requires write permission.")]
        public Task<string> Undo()
        {
            return OnUi("undo", true, () =>
            {
                RequireDocument().Undo();
                return "ok";
            });
        }

        [McpServerTool(Name = "redo"), Description("Redo the last undone edit. Requires write permission.")]
        public Task<string> Redo()
        {
            return OnUi("redo", true, () =>
            {
                var doc = RequireDocument();
                if (doc.CanRedo == false)
                    throw new InvalidOperationException("nothing to redo");
                doc.Redo();
                return "ok";
            });
        }

        [McpServerTool(Name = "replace"), Description("Replace every value 'from' with 'to' on one layer ('tile', 'object' or 'block' 0/1) in one undo step. " +
                                                   "width/height 0 = whole map. Requires write permission.")]
        public Task<string> Replace(string layer, int from, int to, int x = 0, int y = 0, int width = 0, int height = 0)
        {
            return OnUi("replace", true, () =>
            {
                var doc = RequireDocument();
                if (layer != "tile" && layer != "object" && layer != "block")
                    throw new ArgumentException("layer must be tile, object or block");

                var right = width > 0 ? x + width - 1 : doc.Width - 1;
                var bottom = height > 0 ? y + height - 1 : doc.Height - 1;
                var cells = new List<(int, int, CellValue)>();
                foreach (var (cx, cy) in MainWindowViewModel.Rect(x, y, right, bottom).Where(c => doc.Map.Contains(c.X, c.Y)))
                {
                    var cell = doc.Get(cx, cy);
                    if (layer == "tile" && cell.Tile == from)
                        cell.Tile = (ushort)to;
                    else if (layer == "object" && cell.Object == from)
                        cell.Object = (ushort)to;
                    else if (layer == "block" && cell.Block == (from != 0))
                        cell.Block = to != 0;
                    else
                        continue;
                    cells.Add((cx, cy, cell));
                }
                return Json(new { changed = cells.Count == 0 ? 0 : doc.Apply(cells) });
            });
        }

        [McpServerTool(Name = "copy_region"), Description("Copy a rectangle into the editor clipboard (the same one as Ctrl+C), relative to its top-left. " +
                                                       "layers: comma list of tile, object, block. spawns: also copy NPCs and warps inside and mob areas fully inside. " +
                                                       "The clipboard survives open_map, so it can be pasted into another map.")]
        public Task<string> CopyRegion(int x, int y, int width, int height, string layers = "tile,object,block", bool spawns = false)
        {
            return OnUi("copy_region", false, () =>
            {
                var doc = RequireDocument();
                var kinds = new HashSet<CopyKind>();
                foreach (var layer in layers.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
                {
                    if (layer == "tile")
                        kinds.Add(CopyKind.Tile);
                    else if (layer == "object")
                        kinds.Add(CopyKind.Object);
                    else if (layer == "block")
                        kinds.Add(CopyKind.Block);
                    else
                        throw new ArgumentException($"unknown layer '{layer}'");
                }

                var right = x + width - 1;
                var bottom = y + height - 1;
                bool Inside(int cx, int cy) => cx >= x && cx <= right && cy >= y && cy <= bottom;
                var npcs = spawns ? doc.Npcs.Where(n => Inside(n.X, n.Y)).ToList() : new List<NpcSpawn>();
                var warps = spawns ? doc.Warps.Where(w => Inside(w.X, w.Y)).ToList() : new List<WarpEntry>();
                var mobs = spawns ? doc.Mobs.Where(m => Inside(m.Left, m.Top) && Inside(m.Right, m.Bottom)).ToList() : new List<MobSpawn>();
                if (npcs.Count > 0)
                    kinds.Add(CopyKind.Npc);
                if (warps.Count > 0)
                    kinds.Add(CopyKind.Warp);
                if (mobs.Count > 0)
                    kinds.Add(CopyKind.Mob);

                var copyCells = kinds.Contains(CopyKind.Tile) || kinds.Contains(CopyKind.Object) || kinds.Contains(CopyKind.Block);
                _editor.Clipboard = new ClipboardContent
                {
                    Width = width,
                    Height = height,
                    Kinds = kinds,
                    Cells = copyCells
                        ? MainWindowViewModel.Rect(x, y, right, bottom).Where(c => doc.Map.Contains(c.X, c.Y)).Select(c => (c.X - x, c.Y - y, doc.Get(c.X, c.Y))).ToList()
                        : new List<(int, int, CellValue)>(),
                    Npcs = npcs.Select(n => new NpcSpawn { Npc = n.Npc, Info = n.Info, X = n.X - x, Y = n.Y - y, Direction = n.Direction }).ToList(),
                    Warps = warps.Select(w => new WarpEntry { X = w.X - x, Y = w.Y - y, Dest = w.Dest, Condition = w.Condition, DestName = w.DestName }).ToList(),
                    Mobs = mobs.Select(m => new MobSpawn
                    {
                        Mob = m.Mob,
                        Info = m.Info,
                        BeginX = m.Left - x,
                        BeginY = m.Top - y,
                        EndX = m.Right - x,
                        EndY = m.Bottom - y,
                        Count = m.Count,
                        Rezen = m.Rezen,
                        Condition = m.Condition,
                    }).ToList(),
                };
                return Json(new { width, height, layers = kinds.Select(k => k.ToString()), npcs = npcs.Count, warps = warps.Count, mobs = mobs.Count });
            });
        }

        [McpServerTool(Name = "paste_region"), Description("Paste the editor clipboard with its top-left at (x, y) in one undo step; the pasted cells become the selection. " +
                                                        "Requires write permission.")]
        public Task<string> PasteRegion(int x, int y)
        {
            return OnUi("paste_region", true, () =>
            {
                RequireDocument();
                if (_editor.Clipboard == null)
                    throw new InvalidOperationException("the clipboard is empty; call copy_region first");
                _editor.PasteAt(x, y);
                return _editor.StatusText;
            });
        }

        [McpServerTool(Name = "object_info"), Description("One object id: stacked height, collision bits (S=1,N=2,W=4,E=8) and blocked sides, " +
                                                       "how many cells of the open map use it, and door.xlsx pairs that contain it.")]
        public Task<string> ObjectInfo(int id)
        {
            return OnUi("object_info", false, () =>
            {
                var assets = _editor.Assets ?? throw new InvalidOperationException("no client resources loaded");
                var sobj = assets.Objects.Find(id) ?? throw new ArgumentException($"object {id} not found (1..{assets.Objects.Count})");
                var doc = _editor.Document;
                var sides = new[] { (2, "north"), (8, "east"), (1, "south"), (4, "west") }.Where(s => (sobj.Collision & s.Item1) != 0).Select(s => s.Item2);
                return Json(new
                {
                    id,
                    height = sobj.Frames.Length,
                    collision = sobj.Collision,
                    blockedSides = sides,
                    fullyBlocked = (sobj.Collision & 0x0F) == 0x0F,
                    usedOnMap = doc == null ? 0 : doc.Map.Objects.Count(o => o == id),
                    doorPairs = _editor.DoorTable.Pairs.Where(p => p.Open == id || p.Close == id).Select(p => new { pair = p.Id, open = p.Open, close = p.Close }),
                });
            });
        }

        [McpServerTool(Name = "render_objects"), Description("PNG of objects placed side by side on one row, drawn like the map (e.g. a door's closed or open objects). " +
                                                          "scale 1-4, nearest neighbor.")]
        public Task<ImageContentBlock> RenderObjects(int[] ids, int scale = 2)
        {
            return OnUi("render_objects", false, () =>
            {
                var assets = _editor.Assets ?? throw new InvalidOperationException("no client resources loaded");
                if (ids == null || ids.Length == 0 || ids.Length > 64)
                    throw new ArgumentException("ids must have 1..64 entries");
                return Png(Thumbnail.Row(assets, ids), Math.Clamp(scale, 1, 4));
            });
        }

        /// <summary>
        /// PNG of the bitmap's pixels, enlarged by an integer factor without smoothing.
        /// </summary>
        private static ImageContentBlock Png(BitmapSource source, int scale)
        {
            var width = source.PixelWidth;
            var height = source.PixelHeight;
            var pixels = new uint[width * height];
            source.CopyPixels(pixels, width * 4, 0);

            var scaled = new uint[width * scale * height * scale];
            for (int y = 0; y < height * scale; y++)
            {
                for (int x = 0; x < width * scale; x++)
                    scaled[y * width * scale + x] = pixels[y / scale * width + x / scale];
            }

            var bitmap = BitmapSource.Create(width * scale, height * scale, 96, 96, PixelFormats.Pbgra32, null, scaled, width * scale * 4);
            var encoder = new PngBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(bitmap));
            using var stream = new MemoryStream();
            encoder.Save(stream);
            return ImageContentBlock.FromBytes(stream.ToArray(), "image/png");
        }

        [McpServerTool(Name = "save"), Description("Save the open map (.map/.block) and changed spawn sheets. Requires write permission.")]
        public Task<string> Save()
        {
            return OnUi("save", true, () =>
            {
                _editor.Save(_editor.Document);
                return _editor.StatusText;
            });
        }

        [McpServerTool(Name = "render_region"), Description("PNG of a map rectangle (cells, max 48x48) with tiles, objects and .block cells tinted red. " +
                                                         "overlays: outline doors (yellow), NPCs (cyan), warps (magenta) and mob areas (green).")]
        public Task<ImageContentBlock> RenderRegion(int x, int y, int width, int height, bool overlays = true)
        {
            return OnUi("render_region", false, () =>
            {
                var doc = RequireDocument();
                var assets = _editor.Assets ?? throw new InvalidOperationException("no client resources loaded");
                width = Math.Clamp(width, 1, 48);
                height = Math.Clamp(height, 1, 48);
                var cell = assets.CellPixels;
                var pw = width * cell;
                var ph = height * cell;
                var pixels = DrawArea(assets, doc.Map, doc.Blocks, x, y, width, height);

                if (overlays)
                {
                    // Cell rectangle in pixels relative to the image, clipped by Outline.
                    void Box(int left, int top, int right, int bottom, uint color)
                    {
                        Outline(pixels, pw, ph, (left - x) * cell, (top - y) * cell, (right - x + 1) * cell - 1, (bottom - y + 1) * cell - 1, color);
                    }

                    foreach (var mob in doc.Mobs)
                        Box(mob.Left, mob.Top, mob.Right, mob.Bottom, 0xFF40E040);
                    foreach (var door in doc.Doors)
                        Box(door.X, door.Y, door.X + door.Width - 1, door.Y, 0xFFFFD700);
                    foreach (var warp in doc.Warps)
                        Box(warp.X, warp.Y, warp.X, warp.Y, 0xFFFF40FF);
                    foreach (var npc in doc.Npcs)
                        Box(npc.X, npc.Y, npc.X, npc.Y, 0xFF40E0FF);
                }

                var bitmap = BitmapSource.Create(pw, ph, 96, 96, PixelFormats.Pbgra32, null, pixels, pw * 4);
                return Png(bitmap, 1);
            });
        }

        /// <summary>
        /// Pixels of a map rectangle: tiles (tile 0 stays black), objects (also those based below the rectangle
        /// that reach into it) and .block cells tinted red.
        /// </summary>
        private static uint[] DrawArea(ClientAssets assets, ServerMap map, BlockFile blocks, int x, int y, int width, int height)
        {
            var cell = assets.CellPixels;
            var pw = width * cell;
            var ph = height * cell;
            var pixels = new uint[pw * ph];
            Array.Fill(pixels, 0xFF000000u);
            for (int cy = y; cy < y + height; cy++)
            {
                for (int cx = x; cx < x + width; cx++)
                {
                    if (map.Contains(cx, cy) && map.Tiles[cy * map.Width + cx] != 0)
                        assets.DrawTile(pixels, pw, ph, (cx - x) * cell, (cy - y) * cell, map.Tiles[cy * map.Width + cx]);
                }
            }
            for (int cy = y; cy < Math.Min(map.Height, y + height + assets.Objects.MaxHeight); cy++)
            {
                for (int cx = x - 1; cx <= x + width; cx++)
                {
                    if (map.Contains(cx, cy))
                        assets.DrawObject(pixels, pw, ph, (cx - x) * cell, (cy - y) * cell, map.Objects[cy * map.Width + cx]);
                }
            }
            for (int cy = y; cy < y + height; cy++)
            {
                for (int cx = x; cx < x + width; cx++)
                {
                    if (map.Contains(cx, cy) == false || blocks.Contains(cx, cy) == false)
                        continue;

                    for (int py = 0; py < cell; py++)
                    {
                        for (int px = 0; px < cell; px++)
                        {
                            var i = ((cy - y) * cell + py) * pw + (cx - x) * cell + px;
                            var c = pixels[i];
                            var r = (((c >> 16) & 0xFF) + 255) / 2;
                            var g = ((c >> 8) & 0xFF) / 2;
                            var b = (c & 0xFF) / 2;
                            pixels[i] = 0xFF000000u | (r << 16) | (g << 8) | b;
                        }
                    }
                }
            }
            return pixels;
        }

        /// <summary>
        /// 2 px rectangle outline; parts outside the image are skipped.
        /// </summary>
        private static void Outline(uint[] pixels, int width, int height, int left, int top, int right, int bottom, uint color)
        {
            for (int py = Math.Max(0, top); py <= Math.Min(height - 1, bottom); py++)
            {
                for (int px = Math.Max(0, left); px <= Math.Min(width - 1, right); px++)
                {
                    if (px - left < 2 || right - px < 2 || py - top < 2 || bottom - py < 2)
                        pixels[py * width + px] = color;
                }
            }
        }
    }
}
