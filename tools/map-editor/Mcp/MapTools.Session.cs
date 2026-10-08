using System.ComponentModel;
using MapEditor.ViewModel;
using ModelContextProtocol.Server;

namespace MapEditor.Mcp
{
    /// <summary>
    /// What the user is looking at: selection, mouse cell and view. These tools change only the editor view, not the
    /// map, so they do not need write permission.
    /// </summary>
    public partial class MapTools
    {
        [McpServerTool(Name = "get_selection"), Description("What the user selected in the editor: selected cells (max 2000, row by row), their bounds, " +
                                                         "the door edit range when the cells are one horizontal run (widened to whole doors), " +
                                                         "and selected spawns as {kind, index} (index into list_spawns).")]
        public Task<string> GetSelection()
        {
            return OnUi("get_selection", false, () =>
            {
                var doc = RequireDocument();
                var cells = _editor.Selection.OrderBy(c => c.Y).ThenBy(c => c.X).ToList();
                var range = _editor.DoorEditRange();
                return Json(new
                {
                    count = cells.Count,
                    cells = cells.Take(2000).Select(c => new[] { c.X, c.Y }),
                    bounds = cells.Count == 0
                        ? null
                        : new
                        {
                            x = cells.Min(c => c.X),
                            y = cells.Min(c => c.Y),
                            width = cells.Max(c => c.X) - cells.Min(c => c.X) + 1,
                            height = cells.Max(c => c.Y) - cells.Min(c => c.Y) + 1,
                        },
                    doorEditRange = range is (int x, int y, int width) ? new { x, y, width } : null,
                    spawns = _editor.SelectedEntities.Select(e => SpawnRef(doc, e)),
                });
            });
        }

        [McpServerTool(Name = "select_cells"), Description("Select a rectangle of cells in the editor so the user sees them. " +
                                                        "mode: replace (default, also clears selected spawns), add or remove. focus: scroll the view to it.")]
        public Task<string> SelectCells(int x, int y, int width = 1, int height = 1, string mode = "replace", bool focus = true)
        {
            return OnUi("select_cells", false, () =>
            {
                RequireDocument();
                var cells = MainWindowViewModel.Rect(x, y, x + width - 1, y + height - 1);
                if (mode == "replace")
                {
                    _editor.ClearSelection();
                    _editor.ChangeSelection(cells, SelectMode.Add);
                }
                else if (mode == "add")
                {
                    _editor.ChangeSelection(cells, SelectMode.Add);
                }
                else if (mode == "remove")
                {
                    _editor.ChangeSelection(cells, SelectMode.Remove);
                }
                else
                {
                    throw new ArgumentException("mode must be replace, add or remove");
                }

                if (focus)
                    _editor.Jump(x + width / 2, y + height / 2);
                return Json(new { selected = _editor.Selection.Count });
            });
        }

        [McpServerTool(Name = "select_spawn"), Description("Select an NPC, mob spawn or warp (kind + index from list_spawns) in the editor; " +
                                                        "its properties show in the side panel. add keeps the current selection. focus: scroll the view to it.")]
        public Task<string> SelectSpawn(string kind, int index, bool add = false, bool focus = true)
        {
            return OnUi("select_spawn", false, () =>
            {
                var entity = FindSpawn(RequireDocument(), kind, index);
                _editor.SelectEntity(entity, add);
                if (focus)
                    _editor.FocusEntity(entity);
                return "ok";
            });
        }

        [McpServerTool(Name = "focus"), Description("Scroll the editor view so (x, y) is visible. Back/forward navigation in the editor records it.")]
        public Task<string> Focus(int x, int y)
        {
            return OnUi("focus", false, () =>
            {
                RequireDocument();
                _editor.Jump(x, y);
                return "ok";
            });
        }

        [McpServerTool(Name = "get_hover"), Description("The cell under the user's mouse on the map and the status bar text for it, or null when the mouse is off the map.")]
        public Task<string> GetHover()
        {
            return OnUi("get_hover", false, () =>
            {
                RequireDocument();
                if (_editor.HoverCell is not (int x, int y))
                    return "null";
                return Json(new { x, y, text = _editor.HoverText });
            });
        }
    }
}
