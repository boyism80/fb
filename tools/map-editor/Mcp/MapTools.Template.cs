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
                {
                    throw new ArgumentException("ids is empty");
                }

                var missing = ids.Where(id => _editor.Maps.Any(m => m.Id == id) == false).ToList();
                if (missing.Count > 0)
                {
                    throw new ArgumentException($"maps not found: {string.Join(", ", missing)}");
                }

                var deleted = _editor.DeleteMaps(ids.Distinct().ToList());
                return Json(new { deleted, declined = deleted == 0, status = _editor.StatusText });
            });
        }
    }
}
