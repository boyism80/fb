using System.ComponentModel;
using MapEditor.Format;
using MapEditor.Table;

namespace MapEditor.Edit
{
    public class WarpIssue : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Checked for deletion in the issue list. Only issues of the open map can be checked (see InOpenMap).
        /// </summary>
        public bool Checked { get; set; }
        public bool InOpenMap { get; set; }

        public int MapId { get; init; }
        public string MapName { get; init; } = "";
        public int X { get; init; }
        public int Y { get; init; }
        public string Dest { get; init; } = "";
        public string DestName { get; init; } = "";

        /// <summary>
        /// True when the server and client rules make the warp unusable; false for findings that depend on how
        /// players arrive (scripts, login positions and teleports are not known to the editor).
        /// </summary>
        public bool Certain { get; init; }
        public string Message { get; init; } = "";

        public string Severity => Certain ? "확실" : "의심";
        public string MapLabel => $"{MapId:000000} {MapName}";
        public string DestLabel => DestName == "" ? Dest : $"{Dest} {DestName}";
    }

    /// <summary>
    /// Walkability of one map as the server sees it: map::blocked, the SObj direction bits used by
    /// map::movable(object, direction), NPCs occupying cells and warps triggering on the cell walked into.
    /// Door cells are treated as open, since players can open them.
    /// </summary>
    public class WarpCheckMap
    {
        // TOP, RIGHT, BOTTOM, LEFT; SObj bits S=1, N=2, W=4, E=8.
        private static readonly (int Dx, int Dy, byte Leave, byte Enter)[] Steps =
        {
            (0, -1, 2, 1),
            (1, 0, 8, 4),
            (0, 1, 1, 2),
            (-1, 0, 4, 8),
        };
        private static readonly string[] Sides = { "아래", "왼쪽", "위", "오른쪽" };

        public int Id { get; init; }
        public string Name { get; init; } = "";
        public ServerMap Map { get; init; }
        public HashSet<(int X, int Y)> Blocks { get; init; }
        public SObjTable Objects { get; init; }
        public HashSet<(int X, int Y)> DoorCells { get; init; }
        public HashSet<(int X, int Y)> NpcCells { get; init; }
        public List<WarpEntry> Warps { get; init; }

        private HashSet<(int X, int Y)> _warpCells;
        private HashSet<(int X, int Y)> WarpCells => _warpCells ??= Warps.Select(w => (w.X, w.Y)).ToHashSet();

        /// <summary>
        /// Why the cell cannot be stood on, or null when it is open.
        /// </summary>
        public string BlockedBy(int x, int y)
        {
            if (Map.Contains(x, y) == false)
                return "맵 밖";

            var i = y * Map.Width + x;
            if (Blocks.Contains((x, y)))
                return ".block";
            else if (Map.Tiles[i] == 0)
                return "타일 0";
            else if (DoorCells.Contains((x, y)) == false && ((Objects.Find(Map.Objects[i])?.Collision ?? 0) & 0x0F) == 0x0F)
                return $"오브젝트 {Map.Objects[i]} 충돌";
            else
                return null;
        }

        private int Edges(int x, int y)
        {
            if (DoorCells.Contains((x, y)))
                return 0;

            return Objects.Find(Map.Objects[y * Map.Width + x])?.Collision ?? 0;
        }

        /// <summary>
        /// Cells reachable on foot from the arrival points. Warp cells end a walk, so they are not expanded.
        /// </summary>
        private HashSet<(int X, int Y)> Reachable(IEnumerable<(int X, int Y)> arrivals)
        {
            var reached = new HashSet<(int X, int Y)>();
            var queue = new Queue<(int X, int Y)>();
            foreach (var arrival in arrivals)
            {
                if (Map.Contains(arrival.X, arrival.Y) && reached.Add(arrival))
                    queue.Enqueue(arrival);
            }

            while (queue.Count > 0)
            {
                var (x, y) = queue.Dequeue();
                foreach (var step in Steps)
                {
                    var (nx, ny) = (x + step.Dx, y + step.Dy);
                    if (reached.Contains((nx, ny)) || BlockedBy(nx, ny) != null || NpcCells.Contains((nx, ny)) || WarpCells.Contains((nx, ny)))
                        continue;

                    if ((Edges(x, y) & step.Leave) != 0 || (Edges(nx, ny) & step.Enter) != 0)
                        continue;

                    reached.Add((nx, ny));
                    queue.Enqueue((nx, ny));
                }
            }
            return reached;
        }

        /// <summary>
        /// Problems of every warp on this map. arrivals are the destination cells of warps from any map into this one;
        /// destination returns another map (null when its file is missing).
        /// </summary>
        public List<WarpIssue> Check(IReadOnlyList<(int X, int Y)> arrivals, Func<int, WarpCheckMap> destination, Func<int, string> mapName)
        {
            var issues = new List<WarpIssue>();
            var reached = arrivals.Count > 0 ? Reachable(arrivals) : null;
            foreach (var warp in Warps)
            {
                WarpIssue Issue(bool certain, string message) => new WarpIssue
                {
                    MapId = Id,
                    MapName = Name,
                    X = warp.X,
                    Y = warp.Y,
                    Dest = warp.Dest,
                    DestName = warp.DestMap is int destId ? mapName(destId) : "",
                    Certain = certain,
                    Message = message,
                };

                var blocked = BlockedBy(warp.X, warp.Y);
                if (blocked == "맵 밖")
                {
                    issues.Add(Issue(true, "출발 칸이 맵 밖"));
                }
                else if (blocked != null)
                {
                    issues.Add(Issue(true, $"워프 칸이 막혀 있어 들어갈 수 없음 ({blocked})"));
                }
                else if (NpcCells.Contains((warp.X, warp.Y)))
                {
                    issues.Add(Issue(true, "NPC가 워프 칸을 차지함"));
                }
                else
                {
                    // The server warps on a move packet whose forward cell is the warp, so the player must stand on a
                    // neighbor and step into the warp.
                    var entries = new List<(int X, int Y)>();
                    var reasons = new List<string>();
                    for (int d = 0; d < Steps.Length; d++)
                    {
                        var step = Steps[d];
                        var (nx, ny) = (warp.X - step.Dx, warp.Y - step.Dy);
                        var reason = BlockedBy(nx, ny);
                        if (reason == null && NpcCells.Contains((nx, ny)))
                            reason = "NPC";
                        else if (reason == null && WarpCells.Contains((nx, ny)))
                            reason = "다른 워프";
                        else if (reason == null && ((Edges(nx, ny) & step.Leave) != 0 || (Edges(warp.X, warp.Y) & step.Enter) != 0))
                            reason = "방향 충돌";

                        if (reason == null)
                            entries.Add((nx, ny));
                        else
                            reasons.Add($"{Sides[d]} {reason}");
                    }

                    if (entries.Count == 0)
                        issues.Add(Issue(true, $"들어갈 수 있는 인접 칸 없음 ({string.Join(", ", reasons)})"));
                    else if (reached != null && entries.All(e => reached.Contains(e) == false))
                        issues.Add(Issue(false, $"이 맵 도착 지점 {arrivals.Count}곳에서 걸어서 갈 수 없음"));
                }

                if (warp.DestMap is int id && warp.DestX is int dx && warp.DestY is int dy)
                {
                    var dest = destination(id);
                    if (dest == null)
                        issues.Add(Issue(true, $"목적지 맵 {id} 파일 없음"));
                    else if (dest.Map.Contains(dx, dy) == false)
                        issues.Add(Issue(true, $"목적지 좌표가 맵 밖 (맵 크기 {dest.Map.Width}x{dest.Map.Height})"));
                    else if (dest.BlockedBy(dx, dy) is string reason)
                        issues.Add(Issue(false, $"목적지 칸이 막혀 있음 ({reason})"));
                    else if (dest.WarpCells.Contains((dx, dy)))
                        issues.Add(Issue(false, "목적지 칸이 다른 워프 위"));
                }
            }
            return issues;
        }
    }
}
