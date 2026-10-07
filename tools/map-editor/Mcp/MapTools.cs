using System.ComponentModel;
using System.IO;
using System.Text.Json;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Asset;
using MapEditor.Edit;
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
    public class MapTools
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

        [McpServerTool(Name = "open_map"), Description("Open a map in the editor. Unsaved changes of the current map are kept only if saved first.")]
        public async Task<string> OpenMap(int id)
        {
            var task = await OnUi("open_map", false, () =>
            {
                if (_editor.CanOpen == false)
                    throw new InvalidOperationException("The editor is loading or no client version is enabled (set Client paths in appsettings.{MAPEDITOR_ENVIRONMENT}.json).");

                var entry = _editor.Maps.FirstOrDefault(m => m.Id == id) ?? throw new InvalidOperationException($"map {id} not found");
                if (_editor.Document != null && _editor.Document.Dirty)
                    throw new InvalidOperationException("The current map has unsaved changes. Call save or ask the user.");
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

        [McpServerTool(Name = "list_spawns"), Description("Spawns of the open map: kind = 'npc', 'mob' or 'warp'.")]
        public Task<string> ListSpawns(string kind)
        {
            return OnUi("list_spawns", false, () =>
            {
                var doc = RequireDocument();
                if (kind == "npc")
                    return Json(doc.Npcs);
                else if (kind == "mob")
                    return Json(doc.Mobs);
                else if (kind == "warp")
                    return Json(doc.Warps);
                else
                    throw new ArgumentException("kind must be npc, mob or warp");
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

        [McpServerTool(Name = "save"), Description("Save the open map (.map/.block) and changed spawn sheets. Requires write permission.")]
        public Task<string> Save()
        {
            return OnUi("save", true, () =>
            {
                _editor.Save();
                return _editor.StatusText;
            });
        }

        [McpServerTool(Name = "render_region"), Description("PNG of a map rectangle (cells, max 48x48) with tiles, objects and blocked cells in red.")]
        public Task<ImageContentBlock> RenderRegion(int x, int y, int width, int height)
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
                var pixels = new uint[pw * ph];
                Array.Fill(pixels, 0xFF000000u);
                for (int cy = y; cy < y + height; cy++)
                {
                    for (int cx = x; cx < x + width; cx++)
                    {
                        if (doc.Map.Contains(cx, cy))
                            assets.DrawTile(pixels, pw, ph, (cx - x) * cell, (cy - y) * cell, doc.Get(cx, cy).Tile);
                    }
                }
                for (int cy = y; cy < Math.Min(doc.Height, y + height + assets.Objects.MaxHeight); cy++)
                {
                    for (int cx = x - 1; cx <= x + width; cx++)
                    {
                        if (doc.Map.Contains(cx, cy))
                            assets.DrawObject(pixels, pw, ph, (cx - x) * cell, (cy - y) * cell, doc.Get(cx, cy).Object);
                    }
                }
                for (int cy = y; cy < y + height; cy++)
                {
                    for (int cx = x; cx < x + width; cx++)
                    {
                        if (doc.Map.Contains(cx, cy) == false || doc.Blocks.Contains(cx, cy) == false)
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

                var bitmap = BitmapSource.Create(pw, ph, 96, 96, PixelFormats.Pbgra32, null, pixels, pw * 4);
                var encoder = new PngBitmapEncoder();
                encoder.Frames.Add(BitmapFrame.Create(bitmap));
                using var stream = new MemoryStream();
                encoder.Save(stream);
                return ImageContentBlock.FromBytes(stream.ToArray(), "image/png");
            });
        }
    }
}
