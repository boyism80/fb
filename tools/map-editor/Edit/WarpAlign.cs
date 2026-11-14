using System.ComponentModel;
using MapEditor.Format;
using MapEditor.Table;

namespace MapEditor.Edit
{
    public readonly struct WarpCellMove
    {
        public int Map { get; init; }
        public int X { get; init; }
        public int Y { get; init; }
        public int NewX { get; init; }
        public int NewY { get; init; }
        public int DestMap { get; init; }
    }

    public readonly struct WarpDestMove
    {
        public int Map { get; init; }
        public int X { get; init; }
        public int Y { get; init; }
        public int OldDestX { get; init; }
        public int OldDestY { get; init; }
        public int NewDestX { get; init; }
        public int NewDestY { get; init; }
    }

    public sealed class WarpAlignProposal : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public bool Checked { get; set; }
        public int MapId { get; init; }
        public string MapName { get; init; } = "";
        public int DestMapId { get; init; }
        public string Cells { get; init; } = "";
        public string ProposedCells { get; init; } = "";
        public string CurrentLandingText { get; init; } = "";
        public string ProposedLanding { get; init; } = "";
        public string Place { get; init; } = "";
        public string Offset { get; init; } = "";
        public string Action { get; init; } = "";
        public string Note { get; init; } = "";
        public string Anchor { get; init; } = "";
        public string Destination { get; init; } = "";
        public bool Blocked { get; init; }
        public string MapLabel => $"{MapId:000000} {MapName}";
        public List<(int X, int Y)> Current { get; init; } = new List<(int X, int Y)>();
        public List<(int X, int Y)> Proposed { get; init; } = new List<(int X, int Y)>();
        public List<(int X, int Y)> CurrentLanding { get; init; } = new List<(int X, int Y)>();
        public List<(int X, int Y)> NewLanding { get; init; } = new List<(int X, int Y)>();
        public List<WarpCellMove> Moves { get; init; } = new List<WarpCellMove>();
        public List<WarpDestMove> Landings { get; init; } = new List<WarpDestMove>();
    }

    public sealed class WarpAlignChange
    {
        public WarpEntry Entry { get; init; }
        public int? X { get; set; }
        public int? Y { get; set; }
        public string Dest { get; set; }
    }

    public sealed class WarpAlignPlan
    {
        public List<WarpAlignProposal> Applied { get; init; } = new List<WarpAlignProposal>();
        public int Skipped { get; init; }
        public List<WarpAlignChange> Changes { get; init; } = new List<WarpAlignChange>();
    }

    public static class WarpAlign
    {
        private static readonly (int X, int Y)[] Dirs = { (0, 1), (0, -1), (1, 0), (-1, 0) };

        private sealed class Link
        {
            public int Map;
            public int X;
            public int Y;
            public int DestMap;
            public int DestX;
            public int DestY;
            public Link Return;
        }

        private sealed class Step
        {
            public WarpAlignProposal Proposal;
            public WarpEntry Entry;
            public int Map;
            public int? NewX;
            public int? NewY;
            public int? NewDestX;
            public int? NewDestY;
            public int DestMap;
        }

        public static List<WarpAlignProposal> Run(IReadOnlyDictionary<int, ServerMap> maps, Func<int, BlockFile> blocks, IReadOnlyDictionary<int, List<WarpEntry>> warps,
                                                  Func<int, string> name, IProgress<(double Done, string Text)> progress, CancellationToken token)
        {
            var flat = new List<Link>();
            var index = new HashSet<(int Map, int X, int Y)>();
            foreach (var (map, list) in warps)
            {
                foreach (var warp in list)
                {
                    if (warp.DestMap is not int destMap || warp.DestX is not int destX || warp.DestY is not int destY)
                    {
                        continue;
                    }

                    flat.Add(new Link { Map = map, X = warp.X, Y = warp.Y, DestMap = destMap, DestX = destX, DestY = destY });
                    index.Add((map, warp.X, warp.Y));
                }
            }

            var back = new Dictionary<(int Map, int Dest), List<Link>>();
            foreach (var warp in flat)
            {
                var key = (warp.Map, warp.DestMap);
                if (back.TryGetValue(key, out var list) == false)
                {
                    list = new List<Link>();
                    back[key] = list;
                }
                list.Add(warp);
            }

            var linked = new List<Link>();
            for (int i = 0; i < flat.Count; i++)
            {
                token.ThrowIfCancellationRequested();
                if (i % 4000 == 0)
                {
                    progress?.Report((0.05 + 0.35 * i / Math.Max(1, flat.Count), "왕복 워프를 짝 맞추는 중..."));
                }

                var ret = Nearest(flat[i], back);
                if (ret == null)
                {
                    continue;
                }

                flat[i].Return = ret;
                linked.Add(flat[i]);
            }

            var buckets = new Dictionary<(int Map, int Dest, int Ox, int Oy), List<Link>>();
            foreach (var warp in linked)
            {
                var key = (warp.Map, warp.DestMap, warp.Return.DestX - warp.X, warp.Return.DestY - warp.Y);
                if (buckets.TryGetValue(key, out var list) == false)
                {
                    list = new List<Link>();
                    buckets[key] = list;
                }
                list.Add(warp);
            }

            var portals = new List<Portal>();
            foreach (var (key, group) in buckets)
            {
                token.ThrowIfCancellationRequested();
                foreach (var run in Cluster(group))
                {
                    var points = run.Select(w => (X: w.X, Y: w.Y)).ToList();
                    var ox = key.Ox;
                    var oy = key.Oy;
                    var land = points.Select(p => (X: p.X + ox, Y: p.Y + oy)).ToList();
                    var inward = points.Select(p => (X: p.X - ox, Y: p.Y - oy)).ToList();
                    var warpObjs = ObjectsAt(maps, key.Map, points);
                    var landObjs = land.Select(p => ObjectAt(maps, key.Map, p.X, p.Y)).ToList();
                    var inwardObjs = inward.Select(p => ObjectAt(maps, key.Map, p.X, p.Y)).ToList();
                    var (place, anchor) = PlaceOf(warpObjs, landObjs, inwardObjs);
                    portals.Add(new Portal
                    {
                        Map = key.Map,
                        DestMap = key.Dest,
                        Offset = (ox, oy),
                        Place = place,
                        Anchor = anchor,
                        Shown = anchor,
                        Run = run,
                        Points = points,
                        Land = land,
                    });
                }
            }

            var onCounts = new Dictionary<string, Dictionary<(int X, int Y), int>>();
            foreach (var portal in portals)
            {
                if (portal.Place != "타일 위" || portal.Anchor.Length == 0)
                {
                    continue;
                }

                var key = Key(Canon(portal.Anchor));
                if (onCounts.TryGetValue(key, out var counts) == false)
                {
                    counts = new Dictionary<(int X, int Y), int>();
                    onCounts[key] = counts;
                }
                counts.TryGetValue(portal.Offset, out var count);
                counts[portal.Offset] = count + 1;
            }

            var result = new List<WarpAlignProposal>();
            for (int i = 0; i < portals.Count; i++)
            {
                token.ThrowIfCancellationRequested();
                if (i % 2000 == 0)
                {
                    progress?.Report((0.45 + 0.55 * i / Math.Max(1, portals.Count), "규칙을 벗어난 워프를 모으는 중..."));
                }

                var portal = portals[i];
                var offset = portal.Offset;
                var step = Math.Abs(offset.X) + Math.Abs(offset.Y);
                var action = "자동 규칙 밖";
                List<(int X, int Y)> newPoints = portal.Points;
                List<(int X, int Y)> newLand = portal.Land;
                var note = "";
                if (portal.Place != "없음" && step == 1 && Dirs.Contains(offset))
                {
                    if (portal.Place == "착지가 타일")
                    {
                        var past = portal.Land.Select(p => (X: p.X + offset.X, Y: p.Y + offset.Y)).ToList();
                        var stoodOn = 0;
                        if (onCounts.TryGetValue(Key(Canon(portal.Anchor)), out var stood))
                        {
                            stoodOn = stood.Values.Sum();
                        }
                        if (stoodOn > 0 && past.All(p => ObjectAt(maps, portal.Map, p.X, p.Y) is ushort obj && obj != 0))
                        {
                            newPoints = portal.Land;
                            newLand = past;
                            action = "워프를 타일 위로, 착지를 그 방향으로 한 칸 더";
                        }
                        else
                        {
                            action = "유지";
                        }
                    }
                    else if (portal.Place == "바깥")
                    {
                        action = "유지";
                    }
                    else if (StripHorizontal(maps, portal.Map, portal.Points) is bool horizontal && AlongStrip(horizontal, offset))
                    {
                        var anchorKey = Key(Canon(portal.Anchor));
                        var facing = PerpendicularMajority(onCounts, anchorKey, horizontal);
                        if (facing is (int X, int Y) target)
                        {
                            newLand = portal.Points.Select(p => (X: p.X + target.X, Y: p.Y + target.Y)).ToList();
                            action = $"착지만 {FmtOffset(target)} 로";
                        }
                        else
                        {
                            action = "유지";
                        }
                    }
                    else
                    {
                        action = "유지";
                    }
                }

                if (action == "유지" || action == "자동 규칙 밖")
                {
                    continue;
                }

                if (Same(newPoints, portal.Points) && Same(newLand, portal.Land))
                {
                    continue;
                }

                var blocked = false;
                var own = portal.Points.ToHashSet();
                if (OffOrBlocked(maps, blocks, portal.Map, newPoints) || OffOrBlocked(maps, blocks, portal.Map, newLand))
                {
                    note = string.IsNullOrEmpty(note) ? "이동 칸이 맵 밖이거나 막혀 있음" : note + " 이동 칸이 맵 밖이거나 막혀 있음";
                    blocked = true;
                }
                else if (Occupied(index, portal.Map, newPoints, own))
                {
                    note = string.IsNullOrEmpty(note) ? "이동 칸에 다른 워프가 있음" : note + " 이동 칸에 다른 워프가 있음";
                    blocked = true;
                }

                var moves = new List<WarpCellMove>();
                var landings = new List<WarpDestMove>();
                for (int c = 0; c < portal.Points.Count; c++)
                {
                    if (portal.Points[c] != newPoints[c])
                    {
                        moves.Add(new WarpCellMove
                        {
                            Map = portal.Map,
                            X = portal.Points[c].X,
                            Y = portal.Points[c].Y,
                            NewX = newPoints[c].Item1,
                            NewY = newPoints[c].Item2,
                            DestMap = portal.DestMap,
                        });
                    }
                    if (portal.Land[c] != newLand[c])
                    {
                        var ret = portal.Run[c].Return;
                        landings.Add(new WarpDestMove
                        {
                            Map = ret.Map,
                            X = ret.X,
                            Y = ret.Y,
                            OldDestX = portal.Land[c].Item1,
                            OldDestY = portal.Land[c].Item2,
                            NewDestX = newLand[c].Item1,
                            NewDestY = newLand[c].Item2,
                        });
                    }
                }

                var destPts = portal.Run.Select(w => (X: w.DestX, Y: w.DestY)).ToList();
                result.Add(new WarpAlignProposal
                {
                    MapId = portal.Map,
                    MapName = name(portal.Map) ?? "",
                    DestMapId = portal.DestMap,
                    Cells = FmtCells(portal.Points),
                    ProposedCells = FmtCells(newPoints),
                    CurrentLandingText = FmtCells(portal.Land),
                    ProposedLanding = FmtCells(newLand),
                    Place = portal.Place,
                    Offset = FmtOffset(offset),
                    Action = action,
                    Note = note,
                    Anchor = FmtAnchor(portal.Shown),
                    Destination = $"{portal.DestMap} {name(portal.DestMap)} {FmtCells(destPts)}".Trim(),
                    Blocked = blocked,
                    Current = portal.Points,
                    Proposed = newPoints.ToList(),
                    CurrentLanding = portal.Land.ToList(),
                    NewLanding = newLand.ToList(),
                    Moves = moves,
                    Landings = landings,
                });
            }

            progress?.Report((1, $"제안 {result.Count}곳"));
            return result.OrderBy(p => p.MapId).ThenBy(p => p.Current[0].Y).ThenBy(p => p.Current[0].X).ToList();
        }

        public static WarpAlignPlan Plan(IReadOnlyList<WarpAlignProposal> chosen, IReadOnlyDictionary<int, List<WarpEntry>> warps, Func<int, ServerMap> mapOf, Func<int, BlockFile> blocksOf)
        {
            var skipped = new HashSet<WarpAlignProposal>();
            var resolved = new List<Step>();
            foreach (var proposal in chosen)
            {
                if (proposal.Blocked)
                {
                    skipped.Add(proposal);
                    continue;
                }

                var steps = new List<Step>();
                var ok = true;
                foreach (var move in proposal.Moves)
                {
                    var entry = Find(warps, move.Map, move.X, move.Y, move.DestMap, null, null);
                    if (entry == null || BadCell(mapOf, blocksOf, move.Map, move.NewX, move.NewY))
                    {
                        ok = false;
                        break;
                    }
                    steps.Add(new Step { Proposal = proposal, Entry = entry, Map = move.Map, NewX = move.NewX, NewY = move.NewY });
                }
                if (ok == false)
                {
                    skipped.Add(proposal);
                    continue;
                }

                foreach (var land in proposal.Landings)
                {
                    var entry = Find(warps, land.Map, land.X, land.Y, proposal.MapId, land.OldDestX, land.OldDestY);
                    if (entry == null || BadCell(mapOf, blocksOf, proposal.MapId, land.NewDestX, land.NewDestY))
                    {
                        ok = false;
                        break;
                    }
                    steps.Add(new Step
                    {
                        Proposal = proposal,
                        Entry = entry,
                        Map = land.Map,
                        NewDestX = land.NewDestX,
                        NewDestY = land.NewDestY,
                        DestMap = proposal.MapId,
                    });
                }
                if (ok == false)
                {
                    skipped.Add(proposal);
                    continue;
                }
                resolved.AddRange(steps);
            }

            var positions = new Dictionary<WarpEntry, (int X, int Y, WarpAlignProposal Proposal)>();
            foreach (var step in resolved.Where(s => s.NewX is int && skipped.Contains(s.Proposal) == false))
            {
                if (positions.TryGetValue(step.Entry, out var prev))
                {
                    if (prev.X != step.NewX || prev.Y != step.NewY)
                    {
                        skipped.Add(step.Proposal);
                        skipped.Add(prev.Proposal);
                    }
                }
                else
                {
                    positions[step.Entry] = (step.NewX.Value, step.NewY.Value, step.Proposal);
                }
            }

            var dests = new Dictionary<WarpEntry, (int X, int Y, WarpAlignProposal Proposal)>();
            foreach (var step in resolved.Where(s => s.NewDestX is int && skipped.Contains(s.Proposal) == false))
            {
                if (dests.TryGetValue(step.Entry, out var prev))
                {
                    if (prev.X != step.NewDestX || prev.Y != step.NewDestY)
                    {
                        skipped.Add(step.Proposal);
                        skipped.Add(prev.Proposal);
                    }
                }
                else
                {
                    dests[step.Entry] = (step.NewDestX.Value, step.NewDestY.Value, step.Proposal);
                }
            }

            var arrivals = new Dictionary<(int Map, int X, int Y), WarpAlignProposal>();
            foreach (var step in resolved.Where(s => s.NewX is int && skipped.Contains(s.Proposal) == false && (s.Entry.X != s.NewX || s.Entry.Y != s.NewY)))
            {
                var key = (step.Map, step.NewX.Value, step.NewY.Value);
                if (arrivals.TryGetValue(key, out var other))
                {
                    if (other != step.Proposal)
                    {
                        skipped.Add(step.Proposal);
                        skipped.Add(other);
                    }
                }
                else
                {
                    arrivals[key] = step.Proposal;
                }
            }

            var again = true;
            while (again)
            {
                again = false;
                var current = resolved.Where(s => skipped.Contains(s.Proposal) == false && s.NewX is int && (s.Entry.X != s.NewX || s.Entry.Y != s.NewY)).ToList();
                var leaving = current.Select(s => s.Entry).ToHashSet();
                foreach (var step in current)
                {
                    if (warps.TryGetValue(step.Map, out var list) == false ||
                        list.Any(w => w != step.Entry && w.X == step.NewX && w.Y == step.NewY && leaving.Contains(w) == false))
                    {
                        skipped.Add(step.Proposal);
                        again = true;
                        break;
                    }
                }
            }

            var merged = new Dictionary<WarpEntry, WarpAlignChange>();
            var applied = new List<WarpAlignProposal>();
            foreach (var proposal in chosen)
            {
                if (skipped.Contains(proposal))
                {
                    continue;
                }
                applied.Add(proposal);
            }
            foreach (var step in resolved.Where(s => skipped.Contains(s.Proposal) == false))
            {
                if (merged.TryGetValue(step.Entry, out var change) == false)
                {
                    change = new WarpAlignChange { Entry = step.Entry };
                    merged[step.Entry] = change;
                }
                if (step.NewX is int x)
                {
                    change.X = x;
                    change.Y = step.NewY;
                }
                if (step.NewDestX is int destX)
                {
                    change.Dest = $"map({step.DestMap}, {destX}, {step.NewDestY})";
                }
            }

            return new WarpAlignPlan { Applied = applied, Skipped = skipped.Count, Changes = merged.Values.ToList() };
        }

        private static WarpEntry Find(IReadOnlyDictionary<int, List<WarpEntry>> warps, int map, int x, int y, int destMap, int? destX, int? destY)
        {
            if (warps.TryGetValue(map, out var list) == false)
            {
                return null;
            }

            return list.FirstOrDefault(w => w.X == x && w.Y == y && w.DestMap == destMap && (destX == null || w.DestX == destX) && (destY == null || w.DestY == destY));
        }

        private static bool BadCell(Func<int, ServerMap> mapOf, Func<int, BlockFile> blocksOf, int map, int x, int y)
        {
            var info = mapOf(map);
            if (info == null || info.Contains(x, y) == false)
            {
                return true;
            }

            return blocksOf(map).Contains(x, y);
        }

        private sealed class Portal
        {
            public int Map;
            public int DestMap;
            public (int X, int Y) Offset;
            public string Place;
            public int[] Anchor;
            public int[] Shown;
            public List<Link> Run;
            public List<(int X, int Y)> Points;
            public List<(int X, int Y)> Land;
        }

        private static Link Nearest(Link warp, Dictionary<(int Map, int Dest), List<Link>> back)
        {
            if (back.TryGetValue((warp.DestMap, warp.Map), out var list) == false)
            {
                return null;
            }

            Link best = null;
            var bestDist = int.MaxValue;
            foreach (var other in list)
            {
                if (Math.Abs(other.X - warp.DestX) + Math.Abs(other.Y - warp.DestY) > 3)
                {
                    continue;
                }

                var dist = Math.Abs(other.DestX - warp.X) + Math.Abs(other.DestY - warp.Y);
                if (dist > 4 || dist >= bestDist)
                {
                    continue;
                }

                bestDist = dist;
                best = other;
            }
            return best;
        }

        private static List<List<Link>> Cluster(List<Link> group)
        {
            var remaining = new HashSet<Link>(group);
            var runs = new List<List<Link>>();
            while (remaining.Count > 0)
            {
                Link seed = null;
                foreach (var item in remaining)
                {
                    if (seed == null || item.X < seed.X || (item.X == seed.X && item.Y < seed.Y))
                    {
                        seed = item;
                    }
                }

                remaining.Remove(seed);
                var run = new List<Link> { seed };
                var changed = true;
                while (changed)
                {
                    changed = false;
                    foreach (var other in remaining.ToList())
                    {
                        var sameRow = other.Y == run[0].Y && run.All(item => item.Y == run[0].Y);
                        var sameCol = other.X == run[0].X && run.All(item => item.X == run[0].X);
                        var touches = run.Any(item => Math.Abs(other.X - item.X) + Math.Abs(other.Y - item.Y) == 1);
                        if (touches && (sameRow || sameCol))
                        {
                            run.Add(other);
                            remaining.Remove(other);
                            changed = true;
                        }
                    }
                }
                run.Sort((a, b) => a.X != b.X ? a.X.CompareTo(b.X) : a.Y.CompareTo(b.Y));
                runs.Add(run);
            }
            return runs;
        }

        private static List<ushort> ObjectsAt(IReadOnlyDictionary<int, ServerMap> maps, int map, List<(int X, int Y)> points)
        {
            var found = new List<ushort>();
            foreach (var (x, y) in points)
            {
                var obj = ObjectAt(maps, map, x, y);
                if (obj is not ushort value)
                {
                    return null;
                }
                found.Add(value);
            }
            return found;
        }

        private static ushort? ObjectAt(IReadOnlyDictionary<int, ServerMap> maps, int map, int x, int y)
        {
            if (maps.TryGetValue(map, out var info) == false || info.Contains(x, y) == false)
            {
                return null;
            }

            return info.Objects[y * info.Width + x];
        }

        private static bool? StripHorizontal(IReadOnlyDictionary<int, ServerMap> maps, int map, List<(int X, int Y)> points)
        {
            if (points.Count >= 2 && points.All(p => p.Y == points[0].Y))
            {
                return true;
            }

            if (points.Count >= 2 && points.All(p => p.X == points[0].X))
            {
                return false;
            }

            var horizontal = points.Any(p => ObjectAt(maps, map, p.X - 1, p.Y) is ushort left && left != 0 || ObjectAt(maps, map, p.X + 1, p.Y) is ushort right && right != 0);
            var vertical = points.Any(p => ObjectAt(maps, map, p.X, p.Y - 1) is ushort up && up != 0 || ObjectAt(maps, map, p.X, p.Y + 1) is ushort down && down != 0);
            if (horizontal == vertical)
            {
                return null;
            }

            return horizontal;
        }

        private static bool AlongStrip(bool horizontal, (int X, int Y) offset)
        {
            return horizontal ? offset.X != 0 : offset.Y != 0;
        }

        private static (int X, int Y)? PerpendicularMajority(Dictionary<string, Dictionary<(int X, int Y), int>> onCounts, string anchor, bool horizontal)
        {
            if (onCounts.TryGetValue(anchor, out var counts) == false)
            {
                return null;
            }

            var usable = counts.Where(p => Dirs.Contains(p.Key) && (horizontal ? p.Key.X == 0 : p.Key.Y == 0)).ToList();
            if (usable.Count == 0)
            {
                return null;
            }

            return usable.OrderByDescending(p => p.Value).ThenBy(p => DirIndex(p.Key)).First().Key;
        }

        private static (string Place, int[] Anchor) PlaceOf(List<ushort> warpObjs, List<ushort?> landObjs, List<ushort?> inwardObjs)
        {
            if (warpObjs != null && warpObjs.Count > 0 && warpObjs.All(obj => obj != 0))
            {
                return ("타일 위", warpObjs.Select(obj => (int)obj).ToArray());
            }
            if (landObjs.Count > 0 && landObjs.All(obj => obj is ushort value && value != 0))
            {
                return ("착지가 타일", landObjs.Select(obj => (int)obj.Value).ToArray());
            }
            if (inwardObjs.Count > 0 && inwardObjs.All(obj => obj is ushort value && value != 0))
            {
                return ("바깥", inwardObjs.Select(obj => (int)obj.Value).ToArray());
            }
            return ("없음", warpObjs?.Select(obj => (int)obj).ToArray() ?? Array.Empty<int>());
        }

        private static bool OffOrBlocked(IReadOnlyDictionary<int, ServerMap> maps, Func<int, BlockFile> blocks, int map, List<(int X, int Y)> points)
        {
            foreach (var (x, y) in points)
            {
                if (maps.TryGetValue(map, out var info) == false || info.Contains(x, y) == false || blocks(map).Contains(x, y))
                {
                    return true;
                }
            }
            return false;
        }

        private static bool Occupied(HashSet<(int Map, int X, int Y)> index, int map, List<(int X, int Y)> points, HashSet<(int X, int Y)> own)
        {
            foreach (var (x, y) in points)
            {
                if (index.Contains((map, x, y)) && own.Contains((x, y)) == false)
                {
                    return true;
                }
            }
            return false;
        }

        private static bool Same(List<(int X, int Y)> left, List<(int X, int Y)> right)
        {
            if (left.Count != right.Count)
            {
                return false;
            }

            for (int i = 0; i < left.Count; i++)
            {
                if (left[i] != right[i])
                {
                    return false;
                }
            }
            return true;
        }

        private static int[] Canon(int[] anchor)
        {
            var flipped = anchor.Reverse().ToArray();
            for (int i = 0; i < anchor.Length; i++)
            {
                if (anchor[i] == flipped[i])
                {
                    continue;
                }
                return anchor[i] < flipped[i] ? anchor : flipped;
            }
            return anchor;
        }

        private static string Key(int[] anchor) => string.Join("/", anchor);

        private static int DirIndex((int X, int Y) offset)
        {
            for (int i = 0; i < Dirs.Length; i++)
            {
                if (Dirs[i] == offset)
                {
                    return i;
                }
            }
            return Dirs.Length;
        }

        private static string Signed(int value) => value.ToString("+0;-0;+0");

        private static string FmtOffset((int X, int Y) offset) => $"({Signed(offset.X)},{Signed(offset.Y)})";

        private static string FmtCells(List<(int X, int Y)> points) => string.Join(" ", points.Select(p => $"({p.X},{p.Y})"));

        private static string FmtAnchor(int[] anchor)
        {
            if (anchor == null || anchor.Length == 0 || anchor.All(obj => obj == 0))
            {
                return "없음";
            }
            return string.Join("/", anchor);
        }
    }
}
