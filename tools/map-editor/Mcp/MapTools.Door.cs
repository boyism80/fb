using System.ComponentModel;
using MapEditor.Table;
using ModelContextProtocol.Server;

namespace MapEditor.Mcp
{
    public class DoorCellInput
    {
        public int Open { get; set; }
        public int Close { get; set; }
    }

    /// <summary>
    /// door.xlsx definitions (door_pair: open/close object per cell, door: pairs left to right). Changes stay in
    /// memory until save_door_table and are not part of the map's undo history.
    /// </summary>
    public partial class MapTools
    {
        [McpServerTool(Name = "door_partners"), Description("Objects that door.xlsx already pairs with 'object', most used first: " +
                                                         "the open objects when objectIsClosed, otherwise the close objects. Use it to guess a door's other state.")]
        public Task<string> DoorPartners(int @object, bool objectIsClosed = true)
        {
            return OnUi("door_partners", false, () => Json(_editor.DoorPartners(@object, objectIsClosed)));
        }

        [McpServerTool(Name = "list_door_models"), Description("door.xlsx door definitions with their pairs. object: only doors with a pair that contains it. " +
                                                            "onMap: how many doors of the open map use the definition.")]
        public Task<string> ListDoorModels(int? @object = null, int limit = 100)
        {
            return OnUi("list_door_models", false, () =>
            {
                var table = _editor.DoorTable;
                var doc = _editor.Document;
                return Json(table.Doors
                    .Select(d => new { model = d, pairs = d.Pairs.Select(table.FindPair).ToList() })
                    .Where(d => @object == null || d.pairs.Any(p => p != null && (p.Open == @object || p.Close == @object)))
                    .OrderBy(d => d.model.Id)
                    .Take(Math.Clamp(limit, 1, 1000))
                    .Select(d => new
                    {
                        door = d.model.Id,
                        width = d.model.Pairs.Count,
                        pairs = d.pairs.Select((p, i) => p == null
                            ? (object)new { pair = d.model.Pairs[i], missing = true }
                            : new { pair = p.Id, open = p.Open, close = p.Close }),
                        onMap = doc?.Doors.Count(m => m.Model == d.model) ?? 0,
                    }));
            });
        }

        [McpServerTool(Name = "door_references"), Description("Every place the door definition is found on any map (server rule; open tabs with unsaved " +
                                                           "edits): map, x, y, opened. 'ready' is false while the index is still being built after start.")]
        public Task<string> DoorReferences(int door, int limit = 500)
        {
            return OnUi("door_references", false, () =>
            {
                var references = _editor.DoorReferences(door);
                return Json(new
                {
                    ready = _editor.DoorReferencesReady,
                    total = references.Count,
                    maps = references.Select(r => r.Map).Distinct().Count(),
                    references = references.Take(Math.Clamp(limit, 1, 5000)).Select(r => new { r.Map, r.MapName, r.X, r.Y, r.Opened }),
                });
            });
        }

        [McpServerTool(Name = "define_door"), Description("Define the door made of cells starting at (x, y) to the right: one {open, close} per cell " +
                                                       "(open == close keeps a frame cell unchanged). Matching door_pair rows and an identical door are reused, " +
                                                       "so doors on other maps keep their definitions. modifyDoor: replace that door's pairs instead " +
                                                       "(every map using it changes). The map's current objects must equal one state. " +
                                                       "When a lower door id shares the current state but not the other one, the map is switched to the other " +
                                                       "state (storedInOtherState = true). When the server rule still would not find the door at (x, y) nothing is " +
                                                       "changed and 'found' tells which door wins. Call save_door_table to write door.xlsx. Requires write permission.")]
        public Task<string> DefineDoor(int x, int y, DoorCellInput[] cells, int? modifyDoor = null,
                                       [Description("with modifyDoor: also rewrite every other place of that door (all maps) from the old to the new " +
                                                    "objects in the same state; open tabs get an undoable edit, other maps are written right away")]
                                       bool updateOtherMaps = false)
        {
            return OnUi("define_door", true, () =>
            {
                var doc = RequireDocument();
                if (cells == null || cells.Length == 0)
                    throw new ArgumentException("cells must not be empty");
                if (doc.Map.Contains(x, y) == false || doc.Map.Contains(x + cells.Length - 1, y) == false)
                    throw new ArgumentException("the door does not fit in the map");

                var modify = modifyDoor == null
                    ? null
                    : _editor.DoorTable.Doors.FirstOrDefault(d => d.Id == modifyDoor) ?? throw new ArgumentException($"door {modifyDoor} not found");
                var input = cells.Select(c => (c.Open, c.Close)).ToList();
                var before = modify?.Pairs.Select(id => _editor.DoorTable.FindPair(id)).Where(p => p != null).Select(p => (p.Open, p.Close)).ToList();
                var others = modify == null
                    ? new List<DoorReference>()
                    : _editor.DoorReferences(modify.Id).Where(r => (r.Map == doc.Id && r.X == x && r.Y == y) == false).ToList();
                var (model, found, saved, flipped) = _editor.ApplyDoorEdit(x, y, input, modify);

                object otherMaps = null;
                if (saved && modify != null && before.SequenceEqual(input) == false)
                {
                    if (_editor.DoorReferencesReady == false)
                    {
                        otherMaps = new { error = "the door reference index is still being built; nothing else was changed" };
                    }
                    else if (updateOtherMaps)
                    {
                        var (changed, maps, skipped) = _editor.RewriteDoorReferences(before, input, others);
                        otherMaps = new { places = others.Count, changed, maps, skipped };
                    }
                    else
                    {
                        otherMaps = new { places = others.Count, changed = 0, note = "pass updateOtherMaps = true to rewrite them" };
                    }
                }
                return Json(new
                {
                    saved,
                    storedInOtherState = flipped,
                    otherMaps,
                    door = saved ? model.Id : (int?)null,
                    found = found == null ? null : new { door = found.Model.Id, found.X, found.Y, found.Width, found.Opened },
                    doorTableDirty = _editor.DoorTableDirty,
                });
            });
        }

        [McpServerTool(Name = "save_door_table"), Description("Write door.xlsx with the current door definitions. Requires write permission.")]
        public Task<string> SaveDoorTable()
        {
            return OnUi("save_door_table", true, () =>
            {
                _editor.SaveDoorTableCommand.Execute(null);
                return Json(new { status = _editor.StatusText, dirty = _editor.DoorTableDirty });
            });
        }
    }
}
