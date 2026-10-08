using System.IO;
using System.Windows.Threading;
using MapEditor.Edit;
using MapEditor.Format;
using MapEditor.Table;

namespace MapEditor.ViewModel
{
    public partial class MainWindowViewModel
    {
        /// <summary>
        /// Every door on every map by map id, found with the server rule. Open tabs count with their unsaved edits.
        /// </summary>
        private Dictionary<int, List<DoorReference>> _doorReferences = new Dictionary<int, List<DoorReference>>();
        private DispatcherTimer _doorReferenceTimer;
        private int _doorReferenceRevision;

        public bool DoorReferencesReady { get; private set; }

        /// <summary>
        /// Door id → number of places over all maps.
        /// </summary>
        public Dictionary<int, int> DoorReferenceCounts { get; private set; } = new Dictionary<int, int>();

        /// <summary>
        /// Where the selected door definition is used on every map.
        /// </summary>
        public List<DoorReference> SelectedDoorReferences => SelectedDoorModel == null ? new List<DoorReference>() : DoorReferences(SelectedDoorModel.Id);

        public string SelectedDoorReferencesHeader => DoorReferencesReady
            ? $"선택한 문이 쓰이는 곳: 전체 맵 {SelectedDoorReferences.Select(r => r.Map).Distinct().Count()}개, {SelectedDoorReferences.Count}곳 (클릭: 이동)"
            : "선택한 문이 쓰이는 곳: 전체 맵 확인 중...";

        public List<DoorReference> DoorReferences(int door)
        {
            return _doorReferences.Values.SelectMany(list => list.Where(r => r.Door == door))
                                  .OrderBy(r => r.Map).ThenBy(r => r.Y).ThenBy(r => r.X)
                                  .ToList();
        }

        /// <summary>
        /// Rebuilds the index shortly after the last call, so a burst of definition edits scans the maps once.
        /// </summary>
        private void ScheduleDoorReferences()
        {
            if (_doorReferenceTimer == null)
            {
                _doorReferenceTimer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(400) };
                _doorReferenceTimer.Tick += (s, e) =>
                {
                    _doorReferenceTimer.Stop();
                    _ = RefreshDoorReferences();
                };
            }
            _doorReferenceTimer.Stop();
            _doorReferenceTimer.Start();
        }

        private async Task RefreshDoorReferences()
        {
            if (DoorTable == null)
                return;

            var revision = ++_doorReferenceRevision;
            var table = DoorTable.Copy();
            var names = Maps.ToDictionary(m => m.Id, m => m.Name);
            var open = OpenDocuments.ToDictionary(d => d.Id, d => (d.Width, d.Height, Objects: (ushort[])d.Map.Objects.Clone()));
            try
            {
                var corpus = await MapCorpus.Load(Settings.MapDirectory);
                var references = await Task.Run(() =>
                {
                    List<(int Id, int Width, int Height, ushort[] Objects)> maps;
                    lock (corpus)
                        maps = corpus.Select(m => (m.Key, m.Value.Width, m.Value.Height, m.Value.Objects)).ToList();
                    foreach (var (id, map) in open)
                    {
                        var index = maps.FindIndex(m => m.Id == id);
                        if (index >= 0)
                            maps[index] = (id, map.Width, map.Height, map.Objects);
                        else
                            maps.Add((id, map.Width, map.Height, map.Objects));
                    }

                    var result = new System.Collections.Concurrent.ConcurrentDictionary<int, List<DoorReference>>();
                    Parallel.ForEach(maps, map =>
                    {
                        var name = names.TryGetValue(map.Id, out var n) ? n : "";
                        var doors = table.Find(map.Width, map.Height, map.Objects);
                        if (doors.Count > 0)
                            result[map.Id] = doors.Select(d => new DoorReference { Door = d.Model.Id, Map = map.Id, MapName = name, X = d.X, Y = d.Y, Opened = d.Opened }).ToList();
                    });
                    return new Dictionary<int, List<DoorReference>>(result);
                });
                if (revision != _doorReferenceRevision)
                    return;

                _doorReferences = references;
                DoorReferencesReady = true;
                OnDoorReferencesChanged();
            }
            catch (Exception e)
            {
                StatusText = $"문 참조 목록을 만들지 못했습니다: {e.Message}";
            }
        }

        /// <summary>
        /// Replaces the open map's entries after an edit, without scanning the other maps.
        /// </summary>
        private void UpdateDoorReferences(MapDocument document)
        {
            if (DoorReferencesReady == false)
                return;

            var name = Maps.FirstOrDefault(m => m.Id == document.Id)?.Name ?? document.Name;
            _doorReferences[document.Id] = document.Doors.Select(d => new DoorReference { Door = d.Model.Id, Map = document.Id, MapName = name, X = d.X, Y = d.Y, Opened = d.Opened }).ToList();
            OnDoorReferencesChanged();
        }

        private void OnDoorReferencesChanged()
        {
            DoorReferenceCounts = _doorReferences.Values.SelectMany(list => list).GroupBy(r => r.Door).ToDictionary(g => g.Key, g => g.Count());
            OnPropertyChanged(nameof(SelectedDoorReferences));
            OnPropertyChanged(nameof(SelectedDoorReferencesHeader));
        }

        /// <summary>
        /// Opens the map of a door reference (or switches to its tab), selects the door and centers it.
        /// </summary>
        public async Task GoToDoorReference(DoorReference reference)
        {
            if (reference == null)
                return;

            if (Document?.Id == reference.Map)
            {
                Jump(reference.X, reference.Y);
            }
            else
            {
                await OpenMap(Maps.FirstOrDefault(m => m.Id == reference.Map));
                if (Document?.Id != reference.Map)
                    return;
                FocusRequested?.Invoke(reference.X, reference.Y);
            }
            SelectedMapDoor = Document.Doors.FirstOrDefault(d => d.X == reference.X && d.Y == reference.Y);
        }

        /// <summary>
        /// After a door definition changed from before to after (same width), rewrites the given places that still hold
        /// the old objects to the new objects in the same state. Open tabs get an undoable edit that is saved with the tab;
        /// other maps are written right away (.bak kept). Skipped lists places left alone and why.
        /// </summary>
        public (int Changed, int Maps, List<string> Skipped) RewriteDoorReferences(IReadOnlyList<(int Open, int Close)> before, IReadOnlyList<(int Open, int Close)> after,
                                                                                    IReadOnlyCollection<DoorReference> references)
        {
            var skipped = new List<string>();
            if (before.Count != after.Count)
            {
                skipped.AddRange(references.Select(r => $"{r.Label}: 문 폭이 {before.Count}칸에서 {after.Count}칸으로 바뀌어 자동으로 바꿀 수 없음"));
                return (0, 0, skipped);
            }

            var changed = 0;
            var maps = 0;
            foreach (var group in references.GroupBy(r => r.Map))
            {
                var document = OpenDocuments.FirstOrDefault(d => d.Id == group.Key);
                var path = Path.Combine(Settings.MapDirectory, $"{group.Key:000000}.map");
                ServerMap file = null;
                try
                {
                    file = document == null ? ServerMap.Read(path) : null;
                }
                catch (Exception e)
                {
                    skipped.AddRange(group.Select(r => $"{r.Label}: 맵 파일을 읽지 못함 ({e.Message})"));
                    continue;
                }

                var width = document?.Width ?? file.Width;
                var objects = document?.Map.Objects ?? file.Objects;
                var cells = new List<(int X, int Y, ushort Object)>();
                var places = 0;
                foreach (var reference in group)
                {
                    var old = reference.Opened ? before.Select(c => c.Open) : before.Select(c => c.Close);
                    var current = Enumerable.Range(0, before.Count).Select(i => (int)objects[reference.Y * width + reference.X + i]);
                    if (current.SequenceEqual(old) == false)
                    {
                        skipped.Add($"{reference.Label}: 이미 다른 오브젝트로 바뀌어 있음");
                        continue;
                    }

                    var next = reference.Opened ? after.Select(c => c.Open).ToList() : after.Select(c => c.Close).ToList();
                    for (int i = 0; i < next.Count; i++)
                        cells.Add((reference.X + i, reference.Y, (ushort)next[i]));
                    places++;
                }
                if (cells.Count == 0)
                    continue;

                if (document != null)
                {
                    document.Apply(cells.Select(c =>
                    {
                        var value = document.Get(c.X, c.Y);
                        value.Object = c.Object;
                        return (c.X, c.Y, value);
                    }).ToList());
                    document.Doors = DoorTable.Find(document.Width, document.Height, document.Map.Objects);
                }
                else
                {
                    foreach (var (x, y, obj) in cells)
                        file.Objects[y * file.Width + x] = obj;
                    try
                    {
                        MapDocument.WriteAtomic(path, file.ToBytes());
                        MapCorpus.Update(Settings.MapDirectory, group.Key, file);
                    }
                    catch (Exception e)
                    {
                        skipped.Add($"{group.Key:000000}: 맵 파일을 쓰지 못함, {places}곳 그대로 ({e.Message})");
                        continue;
                    }
                }
                changed += places;
                maps++;
            }
            RefreshDoorModels();
            ScheduleDoorReferences();
            return (changed, maps, skipped);
        }
    }
}
