using System.ComponentModel;
using MapEditor.Format;
using MapEditor.Table;

namespace MapEditor.Edit
{
    public class TemplateWarpIssue : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public bool Checked { get; set; }
        public bool InOpenMap { get; set; }

        public int MapId { get; init; }
        public string MapName { get; init; } = "";
        public int X { get; init; }
        public int Y { get; init; }
        public string Dest { get; init; } = "";
        public MapTemplate Template { get; init; }
        public string TemplateName => Template?.Name ?? "";
        public int InstanceX { get; init; }
        public int InstanceY { get; init; }

        /// <summary>
        /// "위치 어긋남" (the building lacks a usual warp cell close by, so this one probably belongs there) or
        /// "다른 곳에 없음" (no counterpart elsewhere and no usual cell close by to move it to).
        /// </summary>
        public string Kind { get; set; } = "";

        /// <summary>
        /// Where the warp would sit if it used the offset most instances of the template use; the warp's own cell
        /// for an extra warp.
        /// </summary>
        public int SuggestX { get; set; }
        public int SuggestY { get; set; }
        public string Message { get; set; } = "";

        public bool CanMove => SuggestX != X || SuggestY != Y;
        public string MapLabel => $"{MapId:000000} {MapName}";
        public string SuggestLabel => CanMove ? $"({SuggestX}, {SuggestY})" : "-";
    }

    /// <summary>
    /// Warps that belong to template instances (on or right around the building) are compared over every map:
    /// when most instances of a template put a warp at the same offset, the instances that put it elsewhere are
    /// reported.
    /// </summary>
    public static class TemplateWarpCheck
    {
        public static List<TemplateWarpIssue> Run(IReadOnlyDictionary<int, ServerMap> maps, IReadOnlyDictionary<int, List<WarpEntry>> warps, TemplateMatcher matcher,
                                                  Func<int, string> mapName, IProgress<(double Done, string Text)> progress, CancellationToken token)
        {
            var ids = warps.Where(w => w.Value.Count > 0 && maps.ContainsKey(w.Key)).Select(w => w.Key).OrderBy(id => id).ToList();
            var links = new List<(TemplateInstance Instance, int Map, WarpEntry Warp)>();
            for (int k = 0; k < ids.Count; k++)
            {
                token.ThrowIfCancellationRequested();
                if (k % 20 == 0)
                    progress.Report((0.9 * k / ids.Count, $"템플릿 찾기: {k}/{ids.Count} 맵, 워프 연결 {links.Count}개"));

                var id = ids[k];
                var instances = matcher.Find(maps[id]);
                if (instances.Count == 0)
                    continue;

                foreach (var warp in warps[id])
                {
                    // On the building, or one cell beside, above or below it (door step).
                    var owner = instances.Where(i => warp.X >= i.X - 1 && warp.X <= i.X + i.Template.Width && warp.Y >= i.Y - 1 && warp.Y <= i.Y + i.Template.Height)
                                         .OrderBy(i => i.Covers(warp.X, warp.Y) ? 0 : 1)
                                         .ThenBy(i => i.Template.Width * i.Template.Height)
                                         .FirstOrDefault();
                    if (owner != null)
                        links.Add((owner, id, warp));
                }
            }

            progress.Report((0.9, $"워프 위치 비교: 연결 {links.Count}개"));
            var issues = new List<TemplateWarpIssue>();
            foreach (var byTemplate in links.GroupBy(l => l.Instance.Template))
            {
                token.ThrowIfCancellationRequested();
                var template = byTemplate.Key;
                var instances = byTemplate.GroupBy(l => (l.Map, l.Instance.X, l.Instance.Y)).ToList();
                if (instances.Count < 2)
                    continue;

                var offsetCounts = instances.SelectMany(g => g.Select(l => (Dx: l.Warp.X - l.Instance.X, Dy: l.Warp.Y - l.Instance.Y)).Distinct())
                                            .GroupBy(o => o)
                                            .ToDictionary(g => g.Key, g => g.Count());
                var common = offsetCounts.Where(o => o.Value >= 2 && o.Value * 2 >= instances.Count).Select(o => o.Key).ToList();
                if (common.Count == 0)
                    continue;

                foreach (var instance in instances)
                {
                    var used = instance.Select(l => (Dx: l.Warp.X - l.Instance.X, Dy: l.Warp.Y - l.Instance.Y)).ToHashSet();
                    var map = maps[instance.Key.Map];
                    var missing = common.Where(c => used.Contains(c) == false && map.Contains(instance.Key.X + c.Dx, instance.Key.Y + c.Dy)).ToList();
                    foreach (var (_, mapId, warp) in instance)
                    {
                        // An offset other instances also use is a variant of the building, not a mistake.
                        var offset = (Dx: warp.X - instance.Key.X, Dy: warp.Y - instance.Key.Y);
                        if (common.Contains(offset) || offsetCounts[offset] >= 2)
                            continue;

                        var near = missing.Where(c => Math.Abs(c.Dx - offset.Dx) + Math.Abs(c.Dy - offset.Dy) <= 3).ToList();
                        var issue = new TemplateWarpIssue
                        {
                            MapId = mapId,
                            MapName = mapName(mapId),
                            X = warp.X,
                            Y = warp.Y,
                            Dest = warp.Dest,
                            Template = template,
                            InstanceX = instance.Key.X,
                            InstanceY = instance.Key.Y,
                        };
                        if (near.Count == 0)
                        {
                            var usual = string.Join(" ", common.Select(c => $"({c.Dx}, {c.Dy})"));
                            issue.Kind = "다른 곳에 없음";
                            issue.SuggestX = warp.X;
                            issue.SuggestY = warp.Y;
                            issue.Message = missing.Count == 0
                                ? $"건물 기준 ({offset.Dx}, {offset.Dy}). 다른 곳은 {usual}에만 워프가 있고 이 건물도 그 자리에 이미 있음"
                                : $"건물 기준 ({offset.Dx}, {offset.Dy}). 다른 곳은 보통 {usual} (3칸 넘게 떨어져 있어 이동 제안 안 함)";
                        }
                        else
                        {
                            var expected = near.OrderBy(c => Math.Abs(c.Dx - offset.Dx) + Math.Abs(c.Dy - offset.Dy)).First();
                            issue.Kind = "위치 어긋남";
                            issue.SuggestX = instance.Key.X + expected.Dx;
                            issue.SuggestY = instance.Key.Y + expected.Dy;
                            issue.Message = $"건물 기준 ({offset.Dx}, {offset.Dy}), 다른 곳은 보통 ({expected.Dx}, {expected.Dy}) [{offsetCounts[expected]}/{instances.Count}곳]";
                        }
                        issues.Add(issue);
                    }
                }
            }
            progress.Report((1, $"완료: 어긋난 워프 {issues.Count}개"));
            return issues.OrderBy(i => i.MapId).ThenBy(i => i.Y).ThenBy(i => i.X).ToList();
        }
    }
}
