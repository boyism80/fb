using MapEditor.Command;
using MapEditor.Edit;
using MapEditor.Format;
using MapEditor.Table;

namespace MapEditor.ViewModel
{
    public partial class MainWindowViewModel
    {
        public event Action WarpAlignRequested;

        public RelayCommand WarpAlignCommand { get; private set; }

        public async Task<List<WarpAlignProposal>> SearchWarpAlign(IProgress<(double Done, string Text)> progress, CancellationToken token)
        {
            if (_spawns == null)
            {
                throw new InvalidOperationException("the editor is still loading");
            }

            var maps = await CorpusSnapshot(progress);
            progress.Report((0.04, "워프 시트 읽는 중..."));
            var spawns = _spawns;
            var warps = await Task.Run(() => spawns.ReadAllWarps(), token);
            foreach (var document in OpenDocuments.ToList())
            {
                warps[document.Id] = document.Warps.ToList();
            }

            var names = _spawns.MapNames;
            var directory = Settings.MapDirectory;
            var openBlocks = OpenDocuments.ToDictionary(d => d.Id, d => d.Blocks);
            BlockFile BlocksOf(int id) => openBlocks.TryGetValue(id, out var file) ? file : MapCorpus.Blocks(directory, id);
            return await Task.Run(() => WarpAlign.Run(maps, BlocksOf, warps, names.Find, progress, token), token);
        }

        public async Task FocusWarpAlign(WarpAlignProposal proposal)
        {
            if (proposal == null || proposal.Current.Count == 0)
            {
                return;
            }

            var x = proposal.Current[0].X;
            var y = proposal.Current[0].Y;
            if (Document?.Id != proposal.MapId)
            {
                var entry = Maps.FirstOrDefault(m => m.Id == proposal.MapId);
                if (entry == null)
                {
                    return;
                }

                await OpenMap(entry);
                if (Document?.Id != proposal.MapId)
                {
                    return;
                }

                SelectedMap = entry;
                FocusRequested?.Invoke(x, y);
            }
            else
            {
                Jump(x, y);
            }

            var warp = Document.Warps.FirstOrDefault(w => w.X == x && w.Y == y);
            if (warp != null)
            {
                SelectEntity(warp, add: false);
            }
        }

        public (ServerMap Map, BlockFile Blocks) WarpAlignView(int mapId)
        {
            var document = OpenDocuments.FirstOrDefault(d => d.Id == mapId);
            if (document != null)
            {
                return (document.Map, document.Blocks);
            }

            var corpus = MapCorpus.Load(Settings.MapDirectory).GetAwaiter().GetResult();
            corpus.TryGetValue(mapId, out var map);
            return (map, MapCorpus.Blocks(Settings.MapDirectory, mapId));
        }

        public async Task<(List<WarpAlignProposal> Applied, int Skipped, string Message)> ApplyWarpAlign(IReadOnlyList<WarpAlignProposal> chosen)
        {
            if (_spawns == null)
            {
                throw new InvalidOperationException("the editor is still loading");
            }

            if (chosen == null || chosen.Count == 0)
            {
                return (new List<WarpAlignProposal>(), 0, "적용할 항목을 체크하세요.");
            }

            var corpus = await MapCorpus.Load(Settings.MapDirectory);
            var warps = _spawns.ReadAllWarps();
            var owners = new Dictionary<WarpEntry, MapDocument>();
            foreach (var document in OpenDocuments.ToList())
            {
                warps[document.Id] = document.Warps.ToList();
                foreach (var warp in document.Warps)
                {
                    owners[warp] = document;
                }
            }

            var directory = Settings.MapDirectory;
            var openMaps = OpenDocuments.ToDictionary(d => d.Id, d => d.Map);
            var openBlocks = OpenDocuments.ToDictionary(d => d.Id, d => d.Blocks);
            ServerMap MapOf(int id) => openMaps.TryGetValue(id, out var map) ? map : corpus.TryGetValue(id, out var stored) ? stored : null;
            BlockFile BlocksOf(int id) => openBlocks.TryGetValue(id, out var file) ? file : MapCorpus.Blocks(directory, id);
            var plan = WarpAlign.Plan(chosen, warps, MapOf, BlocksOf);
            if (plan.Applied.Count == 0)
            {
                var none = plan.Skipped == 0 ? "적용할 항목을 체크하세요." : $"체크한 {chosen.Count}곳은 막혀 있거나 다른 워프가 있거나 이미 바뀌어 건너뜀";
                return (plan.Applied, plan.Skipped, none);
            }

            foreach (var group in plan.Changes.GroupBy(change => owners.TryGetValue(change.Entry, out var document) ? document : null))
            {
                if (group.Key != null)
                {
                    using (group.Key.Group())
                    {
                        foreach (var change in group)
                        {
                            WriteChange(change);
                        }
                    }
                }
                else
                {
                    foreach (var change in group)
                    {
                        WriteChange(change);
                    }
                }
            }

            var mapIds = new HashSet<int>();
            foreach (var proposal in plan.Applied)
            {
                foreach (var move in proposal.Moves)
                {
                    mapIds.Add(move.Map);
                }
                foreach (var landing in proposal.Landings)
                {
                    mapIds.Add(landing.Map);
                }
            }

            var payload = new Dictionary<int, IEnumerable<WarpEntry>>();
            foreach (var id in mapIds)
            {
                var document = OpenDocuments.FirstOrDefault(d => d.Id == id);
                payload[id] = document != null ? document.Warps : warps[id];
            }

            string message;
            try
            {
                _spawns.SaveWarps(payload);
                foreach (var id in mapIds)
                {
                    var document = OpenDocuments.FirstOrDefault(d => d.Id == id);
                    if (document != null)
                    {
                        document.WarpDirty = false;
                    }
                }
                message = $"워프 {plan.Applied.Count}곳 반영" + (plan.Skipped > 0 ? $", {plan.Skipped}곳 건너뜀" : "");
            }
            catch (Exception e)
            {
                message = $"좌표는 바꿨지만 map.xlsx 저장 실패: {e.Message}";
            }

            OverlayInvalidated?.Invoke();
            StatusText = message;
            return (plan.Applied, plan.Skipped, message);
        }

        private static void WriteChange(WarpAlignChange change)
        {
            if (change.X is int x)
            {
                change.Entry.X = x;
            }
            if (change.Y is int y)
            {
                change.Entry.Y = y;
            }
            if (change.Dest != null)
            {
                change.Entry.Dest = change.Dest;
            }
        }
    }
}
