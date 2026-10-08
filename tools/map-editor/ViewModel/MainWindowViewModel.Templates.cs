using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Windows;
using System.Windows.Data;
using System.Windows.Threading;
using MapEditor.Command;
using MapEditor.Edit;
using MapEditor.Format;
using MapEditor.Table;

namespace MapEditor.ViewModel
{
    public partial class MainWindowViewModel
    {
        public event Action TemplateScanRequested;
        public event Action TemplateWarpCheckRequested;

        public ObservableCollection<MapTemplate> Templates { get; } = new ObservableCollection<MapTemplate>();
        public ICollectionView TemplateView { get; private set; }
        public string TemplateQuery { get; set; } = "";
        public MapTemplate SelectedTemplate { get; set; }

        /// <summary>
        /// Templates found on the open map, recomputed when the map, its cells or the templates change.
        /// </summary>
        public List<TemplateInstance> TemplateInstances { get; private set; } = new List<TemplateInstance>();
        private Dictionary<(int X, int Y), TemplateInstance> _templateCells = new Dictionary<(int X, int Y), TemplateInstance>();
        private DispatcherTimer _templateTimer;

        public bool ShowTemplates { get; set; } = true;

        /// <summary>
        /// A click on a cell of a template instance selects the whole instance.
        /// </summary>
        public bool SelectByTemplate { get; set; } = true;

        public string TemplatePath => string.IsNullOrWhiteSpace(User.TemplateFile) ? TemplateFile.DefaultPath : User.TemplateFile;
        public ObservableCollection<TemplateWarpIssue> TemplateWarpIssues { get; } = new ObservableCollection<TemplateWarpIssue>();

        public RelayCommand PlaceTemplateCommand { get; private set; }
        public RelayCommand EditTemplateCommand { get; private set; }
        public RelayCommand<System.Collections.IList> DeleteTemplatesCommand { get; private set; }
        public RelayCommand SaveSelectionAsTemplateCommand { get; private set; }
        public RelayCommand FindTemplateCommand { get; private set; }
        public RelayCommand ScanTemplatesCommand { get; private set; }
        public RelayCommand TemplateWarpCheckCommand { get; private set; }
        public RelayCommand ChangeTemplateFileCommand { get; private set; }
        public RelayCommand MoveTemplateWarpsCommand { get; private set; }
        public RelayCommand FindTemplatePlacesCommand { get; private set; }

        /// <summary>
        /// Every place of TemplatePlacesOf over all maps (open tabs with their unsaved cells and warps).
        /// </summary>
        public List<TemplatePlace> TemplatePlaces { get; private set; } = new List<TemplatePlace>();
        public MapTemplate TemplatePlacesOf { get; private set; }
        public string TemplatePlacesHeader { get; private set; } = "전체 맵 위치: 템플릿을 고르고 [전체 맵에서 찾기]";
        private int _templatePlaceRevision;

        private void InitializeTemplates()
        {
            TemplateView = new ListCollectionView(Templates)
            {
                Filter = item => string.IsNullOrWhiteSpace(TemplateQuery) || ((MapTemplate)item).Name.Contains(TemplateQuery.Trim(), StringComparison.OrdinalIgnoreCase),
            };
            _templateTimer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(300) };
            _templateTimer.Tick += (s, e) =>
            {
                _templateTimer.Stop();
                RefreshTemplateInstances();
            };

            PlaceTemplateCommand = new RelayCommand(_ =>
            {
                if (SelectedTemplate != null)
                    Tool = EditTool.Template;
            });
            EditTemplateCommand = new RelayCommand(_ => EditTemplate(SelectedTemplate));
            DeleteTemplatesCommand = new RelayCommand<System.Collections.IList>(items =>
            {
                var templates = items?.OfType<MapTemplate>().ToList() ?? new List<MapTemplate>();
                if (templates.Count == 0 && SelectedTemplate != null)
                    templates.Add(SelectedTemplate);
                DeleteTemplates(templates);
            });
            SaveSelectionAsTemplateCommand = new RelayCommand(_ => SaveTemplateFrom(Selection.ToList()));
            FindTemplateCommand = new RelayCommand(_ => FindTemplate(SelectedTemplate));
            ScanTemplatesCommand = new RelayCommand(_ => TemplateScanRequested?.Invoke());
            TemplateWarpCheckCommand = new RelayCommand(_ => TemplateWarpCheckRequested?.Invoke());
            ChangeTemplateFileCommand = new RelayCommand(_ => ChangeTemplateFile());
            MoveTemplateWarpsCommand = new RelayCommand(_ => MoveTemplateWarps());
            FindTemplatePlacesCommand = new RelayCommand(_ => _ = FindTemplatePlaces(SelectedTemplate));

            LoadTemplates();
        }

        private void OnTemplateQueryChanged() => TemplateView.Refresh();
        private void OnShowTemplatesChanged() => OverlayInvalidated?.Invoke();
        private void OnSelectedTemplateChanged() => OverlayInvalidated?.Invoke();

        private void LoadTemplates()
        {
            Templates.Clear();
            try
            {
                foreach (var template in TemplateFile.Read(TemplatePath))
                    Templates.Add(template);
                StatusText = $"템플릿 {Templates.Count}개: {TemplatePath}";
            }
            catch (Exception e)
            {
                StatusText = $"템플릿 파일 읽기 실패: {e.Message}";
            }
            RefreshTemplateInstances();
        }

        private bool SaveTemplates()
        {
            try
            {
                TemplateFile.Write(TemplatePath, Templates);
                return true;
            }
            catch (Exception e)
            {
                StatusText = $"템플릿 저장 실패: {e.Message}";
                MessageBox.Show($"템플릿 파일을 저장하지 못했습니다.\n{TemplatePath}\n\n{e.Message}", "템플릿", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
        }

        private void ChangeTemplateFile()
        {
            var dialog = new Microsoft.Win32.SaveFileDialog
            {
                Title = "템플릿 파일 (없는 파일이면 지금 템플릿으로 새로 만듦)",
                Filter = "맵 템플릿 (*.fbt)|*.fbt|모든 파일|*.*",
                FileName = Path.GetFileName(TemplatePath),
                InitialDirectory = Path.GetDirectoryName(TemplatePath),
                OverwritePrompt = false,
            };
            if (dialog.ShowDialog() != true)
                return;

            User.TemplateFile = dialog.FileName;
            OnPropertyChanged(nameof(TemplatePath));
            try
            {
                User.Save();
            }
            catch (Exception e)
            {
                StatusText = $"설정 저장 실패: {e.Message}";
            }

            if (File.Exists(dialog.FileName))
            {
                LoadTemplates();
            }
            else
            {
                if (SaveTemplates())
                    StatusText = $"템플릿 {Templates.Count}개를 새 파일에 저장: {dialog.FileName}";
            }
        }

        [PropertyChanged.SuppressPropertyChangedWarnings]
        private void RefreshTemplateInstances()
        {
            var instances = Document == null || Templates.Count == 0 ? new List<TemplateInstance>() : new TemplateMatcher(Templates, DoorTable.Partners()).Find(Document.Map);
            var cells = new Dictionary<(int X, int Y), TemplateInstance>();
            foreach (var instance in instances)
            {
                foreach (var cell in instance.Cells)
                    cells.TryAdd(cell, instance);
            }
            var counts = instances.GroupBy(i => i.Template).ToDictionary(g => g.Key, g => g.Count());
            foreach (var template in Templates)
                template.OpenMapCount = counts.GetValueOrDefault(template);

            _templateCells = cells;
            TemplateInstances = instances;
            OverlayInvalidated?.Invoke();
        }

        public TemplateInstance TemplateAt(int x, int y) => _templateCells.GetValueOrDefault((x, y));

        public void SelectTemplateInstance(TemplateInstance instance, SelectMode mode)
        {
            if (mode == SelectMode.Replace)
            {
                SelectedEntities.Clear();
                SelectedEntity = null;
            }
            SelectedTemplate = instance.Template;
            ChangeSelection(instance.Cells, mode);
        }

        /// <summary>
        /// Centers the next instance of the template on the open map (cycling) and selects it.
        /// </summary>
        public void FindTemplate(MapTemplate template)
        {
            if (template == null || Document == null)
                return;

            var instances = TemplateInstances.Where(i => i.Template == template).ToList();
            if (instances.Count == 0)
            {
                StatusText = $"열린 맵에 '{template.Name}' 템플릿이 없습니다.";
                return;
            }

            var current = instances.FindIndex(i => i.Cells.All(Selection.Contains) && Selection.Count == i.Template.Cells.Count);
            var next = instances[(current + 1) % instances.Count];
            SelectTemplateInstance(next, SelectMode.Replace);
            Jump(next.X + next.Template.Width / 2, next.Y + next.Template.Height / 2);
            StatusText = $"'{template.Name}' {(current + 1) % instances.Count + 1}/{instances.Count} ({next.X}, {next.Y})";
        }

        /// <summary>
        /// Opens the template editor on the tiles of the cells and the objects on them; the template is added when
        /// the editor is confirmed.
        /// </summary>
        public void SaveTemplateFrom(IReadOnlyCollection<(int X, int Y)> cells)
        {
            if (Document == null || Assets == null)
                return;

            var inside = cells.Where(c => Document.Map.Contains(c.X, c.Y)).ToHashSet();
            if (inside.Count == 0)
            {
                StatusText = "템플릿으로 저장할 칸을 선택하세요.";
                return;
            }

            var template = MapTemplate.FromCells($"{Document.Name} 템플릿", (x, y) => (Document.Get(x, y).Tile, Document.Get(x, y).Object), inside);
            var edited = TemplateEditWindow.Edit(template, Assets, "템플릿 추가");
            if (edited != null)
                AddTemplates(new[] { edited });
        }

        public void AddTemplates(IReadOnlyCollection<MapTemplate> templates)
        {
            var id = Templates.Count == 0 ? 0 : Templates.Max(t => t.Id);
            foreach (var template in templates)
            {
                template.Id = ++id;
                Templates.Add(template);
            }
            SaveTemplates();
            RefreshTemplateInstances();
            if (templates.Count == 1)
            {
                SelectedTemplate = templates.First();
                StatusText = $"템플릿 '{SelectedTemplate.Name}' 추가, 열린 맵에서 {SelectedTemplate.OpenMapCount}곳";
            }
            else
            {
                StatusText = $"템플릿 {templates.Count}개 추가 (전체 {Templates.Count}개)";
            }
        }

        public void EditTemplate(MapTemplate template)
        {
            if (template == null || Assets == null)
                return;

            var edited = TemplateEditWindow.Edit(template.Clone(), Assets, "템플릿 편집");
            if (edited == null)
                return;

            template.Name = edited.Name;
            template.Width = edited.Width;
            template.Height = edited.Height;
            template.Cells = edited.Cells;
            template.Revision++;
            SaveTemplates();
            RefreshTemplateInstances();
            StatusText = $"템플릿 '{template.Name}' 수정, 열린 맵에서 {template.OpenMapCount}곳";
        }

        public int DeleteTemplates(IReadOnlyCollection<MapTemplate> templates)
        {
            if (templates.Count == 0)
                return 0;

            var names = string.Join("\n", templates.Take(15).Select(t => $"  {t.Name}")) + (templates.Count > 15 ? $"\n  ... 외 {templates.Count - 15}개" : "");
            if (MessageBox.Show($"템플릿 {templates.Count}개를 삭제할까요? 맵은 바뀌지 않습니다.\n\n{names}", "템플릿 삭제",
                                MessageBoxButton.YesNo, MessageBoxImage.Warning, MessageBoxResult.No) != MessageBoxResult.Yes)
                return 0;

            foreach (var template in templates.ToList())
                Templates.Remove(template);
            if (SelectedTemplate != null && Templates.Contains(SelectedTemplate) == false)
                SelectedTemplate = null;
            if (Tool == EditTool.Template && SelectedTemplate == null)
                Tool = EditTool.Select;
            if (TemplatePlacesOf != null && Templates.Contains(TemplatePlacesOf) == false)
            {
                _templatePlaceRevision++;
                TemplatePlacesOf = null;
                TemplatePlaces = new List<TemplatePlace>();
                TemplatePlacesHeader = "전체 맵 위치: 템플릿을 고르고 [전체 맵에서 찾기]";
            }
            SaveTemplates();
            RefreshTemplateInstances();
            StatusText = $"템플릿 {templates.Count}개 삭제";
            return templates.Count;
        }

        /// <summary>
        /// Writes the template with its top-left at (x, y) as one undo step; layers a cell does not carry stay as
        /// they are. The placed cells become the selection.
        /// </summary>
        public int PlaceTemplate(MapTemplate template, int x, int y)
        {
            if (Document == null || template == null)
                return 0;

            var cells = new List<(int X, int Y, CellValue Value)>();
            foreach (var cell in template.Cells)
            {
                var cx = x + cell.Dx;
                var cy = y + cell.Dy;
                if (Document.Map.Contains(cx, cy) == false)
                    continue;

                var value = Document.Get(cx, cy);
                if (cell.Tile != null)
                    value.Tile = cell.Tile.Value;
                if (cell.Object != null)
                    value.Object = cell.Object.Value;
                cells.Add((cx, cy, value));
            }
            var changed = cells.Count == 0 ? 0 : Document.Apply(cells);
            ChangeSelection(cells.Select(c => (c.X, c.Y)), SelectMode.Replace);
            StatusText = $"템플릿 '{template.Name}' 배치 ({x}, {y}): {changed}칸 변경";

            // The server finds doors row by row in id order, so the cells around the template can turn its doors
            // into other doors or none.
            var problems = new List<string>();
            foreach (var expected in DoorTable.Find(template.Width, template.Height, template.ObjectGrid()))
            {
                var ex = x + expected.X;
                var ey = y + expected.Y;
                if (Document.Doors.Any(d => d.Model == expected.Model && d.X == ex && d.Y == ey))
                    continue;

                var actual = Document.Doors.FirstOrDefault(d => d.Y == ey && ex + expected.Width > d.X && d.X + d.Width > ex);
                problems.Add(actual == null ? $"문 {expected.Model.Id} ({ex}, {ey}) 인식 안 됨" : $"문 {expected.Model.Id} ({ex}, {ey})이 문 {actual.Model.Id} ({actual.X}, {actual.Y})으로 인식됨");
            }
            if (problems.Count > 0)
                StatusText += $" | 문 확인: {string.Join(", ", problems)}";
            return changed;
        }

        /// <summary>
        /// Every map in the corpus, with the open tabs' unsaved cells instead of their files.
        /// </summary>
        private async Task<Dictionary<int, ServerMap>> CorpusSnapshot(IProgress<(double Done, string Text)> progress)
        {
            progress.Report((0, "맵 파일 읽는 중 (처음 한 번만 몇 초 걸림)..."));
            var corpus = await MapCorpus.Load(Settings.MapDirectory);
            Dictionary<int, ServerMap> maps;
            lock (corpus)
                maps = new Dictionary<int, ServerMap>(corpus);

            foreach (var document in OpenDocuments)
            {
                var copy = new ServerMap(document.Width, document.Height);
                Array.Copy(document.Map.Tiles, copy.Tiles, copy.Tiles.Length);
                Array.Copy(document.Map.Objects, copy.Objects, copy.Objects.Length);
                maps[document.Id] = copy;
            }
            return maps;
        }

        /// <summary>
        /// Lists every place of the template over all maps with the warps around each, for checking them by hand.
        /// Only this template is matched, so places another template would claim are listed too.
        /// </summary>
        public async Task FindTemplatePlaces(MapTemplate template)
        {
            if (template == null || _spawns == null)
                return;

            var revision = ++_templatePlaceRevision;
            TemplatePlacesOf = template;
            TemplatePlaces = new List<TemplatePlace>();
            TemplatePlacesHeader = $"'{template.Name}' 전체 맵에서 찾는 중...";
            try
            {
                var maps = await CorpusSnapshot(new Progress<(double, string)>(_ => { }));
                var spawns = _spawns;
                var openWarps = OpenDocuments.ToDictionary(d => d.Id, d => d.Warps.Select(w => (w.X, w.Y)).ToList());
                var matcher = new TemplateMatcher(new[] { template }, DoorTable.Partners());
                var places = await Task.Run(() =>
                {
                    var warps = spawns.ReadAllWarps().ToDictionary(w => w.Key, w => w.Value.Select(e => (e.X, e.Y)).ToList());
                    foreach (var (id, list) in openWarps)
                        warps[id] = list;

                    var result = new System.Collections.Concurrent.ConcurrentBag<TemplatePlace>();
                    Parallel.ForEach(maps, map =>
                    {
                        foreach (var instance in matcher.Find(map.Value))
                        {
                            var near = warps.GetValueOrDefault(map.Key, new List<(int X, int Y)>())
                                            .Where(w => w.X >= instance.X - 1 && w.X <= instance.X + template.Width && w.Y >= instance.Y - 1 && w.Y <= instance.Y + template.Height)
                                            .Select(w => (w.X - instance.X, w.Y - instance.Y))
                                            .OrderBy(w => w.Item2).ThenBy(w => w.Item1)
                                            .ToList();
                            result.Add(new TemplatePlace { Map = map.Key, MapName = spawns.MapNames.Find(map.Key), X = instance.X, Y = instance.Y, Warps = near });
                        }
                    });
                    return result.OrderBy(p => p.Map).ThenBy(p => p.Y).ThenBy(p => p.X).ToList();
                });
                if (revision != _templatePlaceRevision)
                    return;

                TemplatePlaces = places;
                TemplatePlacesHeader = $"'{template.Name}' 전체 맵 {places.Select(p => p.Map).Distinct().Count()}개, {places.Count}곳 (클릭: 이동, 워프는 건물 기준 좌표)";
            }
            catch (Exception e)
            {
                if (revision == _templatePlaceRevision)
                    TemplatePlacesHeader = $"템플릿 위치를 찾지 못했습니다: {e.Message}";
            }
        }

        /// <summary>
        /// Opens the map of the place (or switches to its tab), centers the building and selects its cells.
        /// </summary>
        public async Task GoToTemplatePlace(TemplatePlace place)
        {
            if (place == null || TemplatePlacesOf == null)
                return;

            var template = TemplatePlacesOf;
            var centerX = place.X + template.Width / 2;
            var centerY = place.Y + template.Height / 2;
            if (Document?.Id == place.Map)
            {
                Jump(centerX, centerY);
            }
            else
            {
                await OpenMap(Maps.FirstOrDefault(m => m.Id == place.Map));
                if (Document?.Id != place.Map)
                    return;
                FocusRequested?.Invoke(centerX, centerY);
            }
            SelectTemplateInstance(new TemplateInstance { Template = template, X = place.X, Y = place.Y }, SelectMode.Replace);
            StatusText = $"'{template.Name}' {place.Label}";
        }

        /// <summary>
        /// Finds repeated object clusters on every map that are not templates yet.
        /// </summary>
        public async Task<List<MapTemplate>> ScanTemplates(TemplateScanOptions options, IProgress<(double Done, string Text)> progress, CancellationToken token)
        {
            if (_spawns == null)
                throw new InvalidOperationException("the editor is still loading");

            var maps = await CorpusSnapshot(progress);
            var doors = DoorTable.Copy();
            var existing = Templates.Select(t => t.Signature(doors)).ToHashSet();
            var names = _spawns.MapNames;
            return await Task.Run(() => TemplateScan.Run(maps, names.Find, options, existing, doors, progress, token), token);
        }

        /// <summary>
        /// Template-based warp position analysis over every map (the open map with its unsaved cells and warps).
        /// </summary>
        public async Task<List<TemplateWarpIssue>> CheckTemplateWarps(IProgress<(double Done, string Text)> progress, CancellationToken token)
        {
            if (_spawns == null)
                throw new InvalidOperationException("the editor is still loading");
            if (Templates.Count == 0)
                throw new InvalidOperationException("등록된 템플릿이 없습니다. 먼저 템플릿을 추가하거나 자동 스캔하세요.");

            var maps = await CorpusSnapshot(progress);
            progress.Report((0, "워프 시트 읽는 중..."));
            var spawns = _spawns;
            var warps = await Task.Run(() => spawns.ReadAllWarps(), token);
            if (Document != null)
                warps[Document.Id] = Document.Warps.ToList();

            var matcher = new TemplateMatcher(Templates.ToList(), DoorTable.Partners());
            var names = _spawns.MapNames;
            var issues = await Task.Run(() => TemplateWarpCheck.Run(maps, warps, matcher, names.Find, progress, token), token);

            TemplateWarpIssues.Clear();
            foreach (var issue in issues)
            {
                issue.InOpenMap = issue.MapId == Document?.Id;
                TemplateWarpIssues.Add(issue);
            }
            return issues;
        }

        public async Task FocusTemplateWarpIssue(TemplateWarpIssue issue)
        {
            if (issue == null)
                return;

            if (Document?.Id != issue.MapId)
            {
                var entry = Maps.FirstOrDefault(m => m.Id == issue.MapId);
                if (entry == null)
                    return;

                await OpenMap(entry);
                if (Document?.Id != issue.MapId)
                    return;
                SelectedMap = entry;
                FocusRequested?.Invoke(issue.X, issue.Y);
            }
            else
            {
                Jump(issue.X, issue.Y);
            }

            foreach (var item in TemplateWarpIssues)
                item.InOpenMap = item.MapId == Document.Id;
            var warp = Document.Warps.FirstOrDefault(w => w.X == issue.X && w.Y == issue.Y && w.Dest == issue.Dest);
            if (warp != null)
                SelectEntity(warp, add: false);
        }

        /// <summary>
        /// Moves the warps of the checked issues on the open map to the suggested cells as one undo step.
        /// </summary>
        public void MoveTemplateWarps()
        {
            if (Document == null)
                return;

            var issues = TemplateWarpIssues.Where(i => i.Checked && i.MapId == Document.Id).ToList();
            if (issues.Count == 0)
            {
                StatusText = "옮길 워프를 체크하세요. 현재 열린 맵의 항목만 옮길 수 있습니다.";
                return;
            }

            var moved = 0;
            var skipped = 0;
            using (Document.Group())
            {
                foreach (var issue in issues)
                {
                    var warp = Document.Warps.FirstOrDefault(w => w.X == issue.X && w.Y == issue.Y && w.Dest == issue.Dest);
                    if (warp == null || issue.CanMove == false || Document.Warps.Any(w => w != warp && w.X == issue.SuggestX && w.Y == issue.SuggestY) || Document.Map.Contains(issue.SuggestX, issue.SuggestY) == false)
                    {
                        skipped++;
                        continue;
                    }
                    warp.X = issue.SuggestX;
                    warp.Y = issue.SuggestY;
                    TemplateWarpIssues.Remove(issue);
                    moved++;
                }
            }
            OverlayInvalidated?.Invoke();
            StatusText = $"워프 {moved}개 이동 (실행 취소 가능){(skipped > 0 ? $", {skipped}개는 추가 워프이거나 워프가 없거나 제안 칸이 차 있어 건너뜀" : "")}";
        }
    }
}
