using System.ComponentModel;
using MapEditor.Edit;
using MapEditor.Table;
using ModelContextProtocol.Server;

namespace MapEditor.Mcp
{
    /// <summary>
    /// NPC, mob and warp spawns of the open map. A spawn is addressed by kind and its index in list_spawns; indexes
    /// shift when a spawn of the same kind is removed. Edits go into the map's undo history and are written by save.
    /// </summary>
    public partial class MapTools
    {
        private static object SpawnRef(MapDocument doc, Entity entity)
        {
            if (entity is NpcSpawn npc)
                return new { kind = "npc", index = doc.Npcs.IndexOf(npc) };
            else if (entity is MobSpawn mob)
                return new { kind = "mob", index = doc.Mobs.IndexOf(mob) };
            else if (entity is WarpEntry warp)
                return new { kind = "warp", index = doc.Warps.IndexOf(warp) };
            else
                return null;
        }

        private static Entity FindSpawn(MapDocument doc, string kind, int index)
        {
            IReadOnlyList<Entity> list = kind switch
            {
                "npc" => doc.Npcs,
                "mob" => doc.Mobs,
                "warp" => doc.Warps,
                _ => throw new ArgumentException("kind must be npc, mob or warp"),
            };
            if (index < 0 || index >= list.Count)
                throw new ArgumentException($"{kind} index {index} out of range (0..{list.Count - 1})");
            return list[index];
        }

        [McpServerTool(Name = "list_spawns"), Description("Spawns of the open map: kind = 'npc', 'mob' or 'warp'. index addresses the spawn in the other spawn tools.")]
        public Task<string> ListSpawns(string kind)
        {
            return OnUi("list_spawns", false, () =>
            {
                var doc = RequireDocument();
                if (kind == "npc")
                    return Json(doc.Npcs.Select((n, i) => new { index = i, npc = n.Npc, name = n.Name, x = n.X, y = n.Y, direction = n.Direction }));
                else if (kind == "mob")
                    return Json(doc.Mobs.Select((m, i) => new { index = i, mob = m.Mob, name = m.Name, left = m.Left, top = m.Top, right = m.Right, bottom = m.Bottom, count = m.Count, rezen = m.Rezen, condition = m.Condition }));
                else if (kind == "warp")
                    return Json(doc.Warps.Select((w, i) => new { index = i, x = w.X, y = w.Y, dest = w.Dest, destName = w.DestName, condition = w.Condition }));
                else
                    throw new ArgumentException("kind must be npc, mob or warp");
            });
        }

        [McpServerTool(Name = "add_npc"), Description("Add an NPC spawn (npc id from npc.xlsx). direction: TOP, RIGHT, BOTTOM or LEFT. Requires write permission.")]
        public Task<string> AddNpc(int npc, int x, int y, string direction = "BOTTOM")
        {
            return OnUi("add_npc", true, () =>
            {
                var doc = RequireDocument();
                if (doc.Map.Contains(x, y) == false)
                    throw new ArgumentException("out of map");
                direction = direction.ToUpperInvariant();
                if (NpcSpawn.Directions.Contains(direction) == false)
                    throw new ArgumentException("direction must be TOP, RIGHT, BOTTOM or LEFT");

                var info = _editor.NpcChoices.FirstOrDefault(e => e.Id == npc) ?? throw new ArgumentException($"npc {npc} is not in npc.xlsx");
                var spawn = new NpcSpawn { Npc = npc, Info = info, X = x, Y = y, Direction = direction };
                doc.Npcs.Add(spawn);
                _editor.SelectEntity(spawn, add: false);
                return Json(new { kind = "npc", index = doc.Npcs.Count - 1, name = info.Name, blocked = _editor.IsBlocked(x, y) });
            });
        }

        [McpServerTool(Name = "add_mob"), Description("Add a mob spawn area (mob id from mob.xlsx, inclusive cell rectangle). rezen is hh:mm:ss. Requires write permission.")]
        public Task<string> AddMob(int mob, int left, int top, int right, int bottom, int count = 1, string rezen = "00:05:00", string condition = "")
        {
            return OnUi("add_mob", true, () =>
            {
                var doc = RequireDocument();
                var info = _editor.MobChoices.FirstOrDefault(e => e.Id == mob) ?? throw new ArgumentException($"mob {mob} is not in mob.xlsx");
                var spawn = new MobSpawn
                {
                    Mob = mob,
                    Info = info,
                    BeginX = Math.Clamp(Math.Min(left, right), 0, doc.Width - 1),
                    BeginY = Math.Clamp(Math.Min(top, bottom), 0, doc.Height - 1),
                    EndX = Math.Clamp(Math.Max(left, right), 0, doc.Width - 1),
                    EndY = Math.Clamp(Math.Max(top, bottom), 0, doc.Height - 1),
                    Count = Math.Max(1, count),
                    Rezen = rezen,
                    Condition = condition,
                };
                doc.Mobs.Add(spawn);
                _editor.SelectEntity(spawn, add: false);
                return Json(new { kind = "mob", index = doc.Mobs.Count - 1, name = info.Name });
            });
        }

        [McpServerTool(Name = "add_warp"), Description("Add a warp at (x, y). dest is the warp sheet text, usually 'map(id, x, y)'. Requires write permission.")]
        public Task<string> AddWarp(int x, int y, string dest, string condition = "")
        {
            return OnUi("add_warp", true, () =>
            {
                var doc = RequireDocument();
                if (doc.Map.Contains(x, y) == false)
                    throw new ArgumentException("out of map");

                var warp = new WarpEntry { X = x, Y = y, Dest = dest, Condition = condition };
                warp.DestName = warp.DestMap is int id ? _editor.MapChoices.FirstOrDefault(e => e.Id == id)?.Name ?? "" : "";
                doc.Warps.Add(warp);
                _editor.SelectEntity(warp, add: false);
                return Json(new { kind = "warp", index = doc.Warps.Count - 1, destName = warp.DestName, blocked = _editor.IsBlocked(x, y) });
            });
        }

        [McpServerTool(Name = "update_spawn"), Description("Change fields of a spawn in one undo step; omitted fields keep their value. " +
                                                        "npc: id, x, y, direction. mob: id, left, top, right, bottom, count, rezen, condition. " +
                                                        "warp: x, y, dest, condition. Requires write permission.")]
        public Task<string> UpdateSpawn(string kind, int index, int? id = null, int? x = null, int? y = null, string direction = null,
                                        int? left = null, int? top = null, int? right = null, int? bottom = null, int? count = null,
                                        string rezen = null, string condition = null, string dest = null)
        {
            return OnUi("update_spawn", true, () =>
            {
                var doc = RequireDocument();
                var entity = FindSpawn(doc, kind, index);
                if (entity is NpcSpawn && id != null && _editor.NpcChoices.Any(e => e.Id == id) == false)
                    throw new ArgumentException($"npc {id} is not in npc.xlsx");
                if (entity is MobSpawn && id != null && _editor.MobChoices.Any(e => e.Id == id) == false)
                    throw new ArgumentException($"mob {id} is not in mob.xlsx");
                if (direction != null && NpcSpawn.Directions.Contains(direction.ToUpperInvariant()) == false)
                    throw new ArgumentException("direction must be TOP, RIGHT, BOTTOM or LEFT");

                using (doc.Group())
                {
                    if (entity is NpcSpawn npc)
                    {
                        npc.Npc = id ?? npc.Npc;
                        npc.Direction = direction?.ToUpperInvariant() ?? npc.Direction;
                        npc.X = x ?? npc.X;
                        npc.Y = y ?? npc.Y;
                    }
                    else if (entity is MobSpawn mob)
                    {
                        mob.Mob = id ?? mob.Mob;
                        var (l, t, r, b) = (left ?? mob.Left, top ?? mob.Top, right ?? mob.Right, bottom ?? mob.Bottom);
                        _editor.SetMobArea(mob, l, t, r, b);
                        mob.Count = count ?? mob.Count;
                        mob.Rezen = rezen ?? mob.Rezen;
                        mob.Condition = condition ?? mob.Condition;
                    }
                    else if (entity is WarpEntry warp)
                    {
                        warp.X = x ?? warp.X;
                        warp.Y = y ?? warp.Y;
                        warp.Dest = dest ?? warp.Dest;
                        warp.Condition = condition ?? warp.Condition;
                    }
                }
                return Json(SpawnRef(doc, entity));
            });
        }

        [McpServerTool(Name = "remove_spawn"), Description("Remove a spawn (kind + index from list_spawns). Indexes of later spawns of that kind shift down by one. Requires write permission.")]
        public Task<string> RemoveSpawn(string kind, int index)
        {
            return OnUi("remove_spawn", true, () =>
            {
                _editor.RemoveEntity(FindSpawn(RequireDocument(), kind, index));
                return "ok";
            });
        }

        [McpServerTool(Name = "check_warps"), Description("Find unusable warps (blocked or occupied warp cell, no enterable neighbor, unreachable from arrival points, " +
                                                       "missing destination map or blocked destination cell). all: every map with warps (takes a while), " +
                                                       "otherwise the open map with its unsaved edits. certain=false entries depend on how players arrive. " +
                                                       "The results also fill the editor's issue window.")]
        public async Task<string> CheckWarps(bool all = false, int limit = 200)
        {
            var task = await OnUi("check_warps", false, () =>
            {
                if (all == false)
                    RequireDocument();
                if (_editor.CheckingWarps)
                    throw new InvalidOperationException("a warp check is already running");
                return _editor.CheckWarps(all);
            });
            await task;
            return await OnUi("check_warps", false, () => Json(new
            {
                status = _editor.StatusText,
                total = _editor.WarpIssues.Count,
                issues = _editor.WarpIssues.Take(Math.Clamp(limit, 1, 5000)).Select(i => new
                {
                    map = i.MapId,
                    mapName = i.MapName,
                    i.X,
                    i.Y,
                    dest = i.Dest,
                    destName = i.DestName,
                    certain = i.Certain,
                    message = i.Message,
                }),
            }));
        }
    }
}
