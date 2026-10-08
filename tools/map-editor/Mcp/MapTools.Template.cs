using System.ComponentModel;
using ModelContextProtocol.Server;

namespace MapEditor.Mcp
{
    public partial class MapTools
    {
        [McpServerTool(Name = "delete_maps"), Description("Delete maps: their map.xlsx rows and npc_spawn / mob_spawn / warp groups are removed and their .map/.block files " +
                                                       "go to the recycle bin. The editor always shows a confirmation dialog to the user; deleted is 0 when the " +
                                                       "user declined. Warps of other maps that lead to them are kept. Requires write permission.")]
        public async Task<string> DeleteMaps(int[] ids)
        {
            return await OnUi("delete_maps", true, () =>
            {
                if (ids == null || ids.Length == 0)
                    throw new ArgumentException("ids is empty");

                var missing = ids.Where(id => _editor.Maps.Any(m => m.Id == id) == false).ToList();
                if (missing.Count > 0)
                    throw new ArgumentException($"maps not found: {string.Join(", ", missing)}");

                var deleted = _editor.DeleteMaps(ids.Distinct().ToList());
                return Json(new { deleted, declined = deleted == 0, status = _editor.StatusText });
            });
        }

        [McpServerTool(Name = "list_templates"), Description("Templates of the user's template file (buildings made of tiles and objects): id, name, size, " +
                                                          "object/tile cell counts and how often each appears on the open map.")]
        public Task<string> ListTemplates(string query = "", int limit = 100)
        {
            return OnUi("list_templates", false, () => Json(_editor.Templates
                .Where(t => string.IsNullOrWhiteSpace(query) || t.Name.Contains(query, StringComparison.OrdinalIgnoreCase))
                .Take(Math.Clamp(limit, 1, 1000))
                .Select(t => new { t.Id, t.Name, t.Auto, t.Width, t.Height, objects = t.ObjectCount, tiles = t.TileCount, onOpenMap = t.OpenMapCount })));
        }

        [McpServerTool(Name = "place_template"), Description("Write a template with its top-left at (x, y) in one undo step. Cells without a tile keep the map's tile, " +
                                                          "cells without an object keep the map's object. Requires write permission.")]
        public Task<string> PlaceTemplate(int id, int x, int y)
        {
            return OnUi("place_template", true, () =>
            {
                RequireDocument();
                var template = _editor.Templates.FirstOrDefault(t => t.Id == id) ?? throw new ArgumentException($"template {id} not found");
                var changed = _editor.PlaceTemplate(template, x, y);
                return Json(new { template = template.Name, x, y, changed });
            });
        }
    }
}
