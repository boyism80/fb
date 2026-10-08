using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Windows;
using System.Windows.Data;
using MapEditor.Asset;
using MapEditor.Command;
using MapEditor.Edit;
using MapEditor.Format;
using MapEditor.Settings;
using MapEditor.Table;

namespace MapEditor.ViewModel
{
    public enum EditTool
    {
        Select,
        Brush,
        Rect,
        Fill,
        Eraser,
        Eyedropper,
        Door,
        Mob,
    }

    public enum SelectMode
    {
        Replace,
        Add,
        Remove,
    }

    public class VersionOption
    {
        public ClientVersion Version { get; init; }
        public bool Enabled { get; init; }
        public string Label => Enabled ? Version.ToString() : $"{Version} (경로 없음)";
    }

    /// <summary>
    /// One line of the loading popup.
    /// </summary>
    public class LoadingStep : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public string Name { get; init; } = "";
        public bool Done { get; set; }
        public bool Failed { get; set; }
        public long Milliseconds { get; set; }

        public string State => Failed ? "실패" : Done ? $"{Milliseconds} ms" : "읽는 중...";
    }

    public class MapEntry
    {
        public int Id { get; init; }
        public string Name { get; init; } = "";
        public string Label => $"{Id:000000} {Name}";
    }

    public enum IssueTab
    {
        Warps,
        Validation,
    }

    public class ValidationItem
    {
        public int X { get; init; }
        public int Y { get; init; }
        public string Message { get; init; } = "";
        public string Label => $"({X}, {Y}) {Message}";
    }

    /// <summary>
    /// A back/forward history entry: map and the view center in cells.
    /// </summary>
    public readonly record struct NavLocation(int Map, double X, double Y);

    public enum CopyKind
    {
        Tile,
        Object,
        Block,
        Npc,
        Warp,
        Mob,
    }

    /// <summary>
    /// Copied cells and spawns, relative to the top-left of what was copied. Kinds says which cell layers to
    /// write when pasting; spawns are pasted as new copies.
    /// </summary>
    public class ClipboardContent
    {
        public int Width { get; init; }
        public int Height { get; init; }
        public HashSet<CopyKind> Kinds { get; init; } = new HashSet<CopyKind>();
        public List<(int Dx, int Dy, CellValue Value)> Cells { get; init; } = new List<(int Dx, int Dy, CellValue Value)>();
        public List<NpcSpawn> Npcs { get; init; } = new List<NpcSpawn>();
        public List<WarpEntry> Warps { get; init; } = new List<WarpEntry>();
        public List<MobSpawn> Mobs { get; init; } = new List<MobSpawn>();
    }

    public class MainWindowViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Baked chunk images must be rebuilt (assets or tile/object visibility changed).
        /// </summary>
        public event Action RenderInvalidated;

        /// <summary>
        /// Only overlays changed (selection, markers, doors).
        /// </summary>
        public event Action OverlayInvalidated;

        /// <summary>
        /// Scroll the canvas so the cell is visible.
        /// </summary>
        public event Action<int, int> FocusRequested;

        /// <summary>
        /// Scroll the canvas so the map point (in cells) is at the middle; used by back/forward navigation.
        /// </summary>
        public event Action<double, double> CenterRequested;

        public event Action MapSearchRequested;
        public event Action ShortcutEditorRequested;

        /// <summary>
        /// Show the issue window on a tab (validation or warp check started from the menu or a shortcut).
        /// </summary>
        public event Action<IssueTab> IssuesRequested;

        /// <summary>
        /// Open the door editor for width cells starting at (x, y).
        /// </summary>
        public event Action<int, int, int> DoorEditRequested;

        private readonly Dictionary<ClientVersion, ClientAssets> _assetCache = new Dictionary<ClientVersion, ClientAssets>();
        private SpawnTable _spawns;
        private (int X, int Y)? _hoverCell;

        /// <summary>
        /// The paste that is still the selection and the newest edit. Moving it re-pastes at the new place instead of
        /// moving cells, so every copied layer follows and the cells underneath come back.
        /// </summary>
        /// Changed is false when the paste matched what was already there (pasting onto the copied area), so there is
        /// nothing to undo before re-pasting.
        private (int X, int Y, object Step, ClipboardContent Content, bool Changed)? _floatingPaste;

        public bool FloatingPaste => _floatingPaste is (_, _, var step, var content, _) && step == Document?.LastStep && content == Clipboard;

        private readonly List<NavLocation> _back = new List<NavLocation>();
        private readonly List<NavLocation> _forward = new List<NavLocation>();

        public AppSettings Settings { get; }
        public UserSettings User { get; }

        /// <summary>
        /// Every action that can be bound to keys or mouse side buttons, in menu order.
        /// </summary>
        public List<ShortcutAction> Shortcuts { get; }

        /// <summary>
        /// Shortcuts by id, for menu InputGestureText bindings.
        /// </summary>
        public Dictionary<string, ShortcutAction> ShortcutMap { get; }

        /// <summary>
        /// Middle of the visible map in cells, kept up to date by the canvas.
        /// </summary>
        [PropertyChanged.DoNotNotify]
        public (double X, double Y) ViewCenter { get; set; }

        public bool CanNavigateBack => _back.Count > 0;
        public bool CanNavigateForward => _forward.Count > 0;
        public List<VersionOption> Versions { get; }
        public VersionOption SelectedVersion { get; set; }
        public ClientAssets Assets { get; private set; }
        public bool Busy { get; private set; }
        public bool CanOpen => Assets != null && Busy == false;

        /// <summary>
        /// Loading popup: shown only when a load is still running after a short delay, so fast loads do not flash.
        /// </summary>
        public ObservableCollection<LoadingStep> LoadingSteps { get; } = new ObservableCollection<LoadingStep>();
        public string LoadingTitle { get; private set; } = "";
        public bool LoadingVisible { get; private set; }
        public int LoadingDone { get; private set; }
        public int LoadingTotal { get; private set; }
        private int _loadingId;

        public ObservableCollection<MapEntry> Maps { get; } = new ObservableCollection<MapEntry>();
        public ICollectionView MapView { get; }
        public string MapQuery { get; set; } = "";
        public MapEntry SelectedMap { get; set; }
        public MapDocument Document { get; private set; }

        public List<NameEntry> NpcChoices => _spawns?.NpcNames.Entries ?? new List<NameEntry>();
        public List<NameEntry> MobChoices => _spawns?.MobNames.Entries ?? new List<NameEntry>();
        public List<NameEntry> MapChoices => _spawns?.MapNames.Entries ?? new List<NameEntry>();

        public EditTool Tool { get; set; } = EditTool.Select;
        public EditLayer Layer { get; set; } = EditLayer.Tile;
        public int SelectedTile { get; set; } = 1;
        public int SelectedObject { get; set; } = 1;
        public double Zoom { get; set; } = 1.0;

        public bool ShowTiles { get; set; } = true;
        public bool ShowObjects { get; set; } = true;
        public bool ShowBlocks { get; set; } = true;
        public bool ShowCollision { get; set; } = false;
        public bool ShowGrid { get; set; } = false;
        public bool ShowDoors { get; set; } = true;
        public bool ShowNpcs { get; set; } = true;
        public bool ShowMobs { get; set; } = true;
        public bool ShowWarps { get; set; } = true;

        /// <summary>
        /// NPC and mob spawns drawn with their client sprites instead of plain markers.
        /// </summary>
        public bool ShowSprites { get; set; } = true;

        public bool ShowMinimap { get; set; } = true;
        public bool MinimapVisible => ShowMinimap && Document != null;

        /// <summary>
        /// With another resource version selected, cells whose tile or object id is beyond the 5.50 resources are
        /// drawn black (what a 5.50 client cannot show). Needs the 5.50 client path.
        /// </summary>
        public bool ShowMissing550 { get; set; } = true;

        /// <summary>
        /// 5.50 tile and object counts while ShowMissing550 applies; null otherwise.
        /// </summary>
        public (int Tiles, int Objects)? Limit550 { get; private set; }

        /// <summary>
        /// Moving a selection always moves objects; these add the tile and block layers.
        /// </summary>
        public bool MoveTiles { get; set; }
        public bool MoveBlocks { get; set; }

        public HashSet<(int X, int Y)> Selection { get; } = new HashSet<(int X, int Y)>();
        public HashSet<Entity> SelectedEntities { get; } = new HashSet<Entity>();
        public string SelectionText { get; private set; } = "선택 없음";
        public bool HasSelection { get; private set; }
        public ClipboardContent Clipboard { get; private set; }

        public string HoverText { get; private set; } = "";
        public string StatusText { get; set; } = "";

        public DoorTable DoorTable { get; private set; }
        public MapDoor SelectedMapDoor { get; set; }
        public DoorModel SelectedDoorModel { get; set; }

        /// <summary>
        /// Door definitions, optionally only those found on the open map.
        /// </summary>
        public ListCollectionView DoorModelsView { get; private set; }
        public bool OnlyMapDoors { get; set; }

        /// <summary>
        /// Where the selected door definition is used on the open map.
        /// </summary>
        public List<MapDoor> SelectedDoorUsages => Document == null || SelectedDoorModel == null
            ? new List<MapDoor>()
            : Document.Doors.Where(d => d.Model == SelectedDoorModel).ToList();
        public DoorPair SelectedDoorPair { get; set; }
        public bool PlaceDoorOpened { get; set; }
        public bool DoorTableDirty { get; private set; }

        /// <summary>
        /// Bumped on every door definition change so door previews rebuild.
        /// </summary>
        public int DoorRevision { get; private set; }

        public NpcSpawn SelectedNpc { get; set; }
        public MobSpawn SelectedMob { get; set; }
        public WarpEntry SelectedWarp { get; set; }

        /// <summary>
        /// The entity shown in the property panel.
        /// </summary>
        public Entity SelectedEntity { get; private set; }

        public ObservableCollection<ValidationItem> Validation { get; } = new ObservableCollection<ValidationItem>();
        public ObservableCollection<WarpIssue> WarpIssues { get; } = new ObservableCollection<WarpIssue>();
        public bool CheckingWarps { get; private set; }

        public string McpStatus { get; set; } = "꺼짐";
        public bool McpAllowWrite { get; set; }
        public ObservableCollection<string> McpLog { get; } = new ObservableCollection<string>();

        public RelayCommand<MapEntry> OpenMapCommand { get; }
        public RelayCommand SaveCommand { get; }
        public RelayCommand UndoCommand { get; }
        public RelayCommand RedoCommand { get; }
        public RelayCommand<string> ToolCommand { get; }
        public RelayCommand<string> LayerCommand { get; }
        public RelayCommand<string> ZoomCommand { get; }
        public RelayCommand<string> VersionCommand { get; }
        public RelayCommand DeleteSelectionCommand { get; }
        public RelayCommand ToggleBlockSelectionCommand { get; }
        public RelayCommand CopyCommand { get; }
        public RelayCommand PasteCommand { get; }
        public RelayCommand SelectAllCommand { get; }
        public RelayCommand ClearSelectionCommand { get; }
        public RelayCommand ToggleDoorCommand { get; }
        public RelayCommand DeleteDoorCommand { get; }
        public RelayCommand EditDoorCommand { get; }
        public RelayCommand AddDoorPairCommand { get; }
        public RelayCommand RemoveDoorPairCommand { get; }
        public RelayCommand AddDoorModelCommand { get; }
        public RelayCommand RemoveDoorModelCommand { get; }
        public RelayCommand SaveDoorTableCommand { get; }
        public RelayCommand<string> AddSpawnCommand { get; }
        public RelayCommand<string> RemoveSpawnCommand { get; }
        public RelayCommand PickNpcCommand { get; }
        public RelayCommand PickMobCommand { get; }
        public RelayCommand PickWarpMapCommand { get; }
        public RelayCommand<string> NpcDirectionCommand { get; }
        public RelayCommand OpenWarpDestinationCommand { get; }
        public RelayCommand DeleteEntityCommand { get; }
        public RelayCommand FocusEntityCommand { get; }
        public RelayCommand ValidateCommand { get; }
        public RelayCommand<ValidationItem> FocusValidationCommand { get; }
        public RelayCommand<MapDoor> FocusDoorCommand { get; }
        public RelayCommand ExitCommand { get; }
        public RelayCommand NavigateBackCommand { get; }
        public RelayCommand NavigateForwardCommand { get; }
        public RelayCommand<string> CheckWarpsCommand { get; }
        public RelayCommand<WarpIssue> FocusWarpIssueCommand { get; }
        public RelayCommand DeleteCheckedWarpsCommand { get; }
        public RelayCommand ShortcutEditorCommand { get; }

        public MainWindowViewModel(AppSettings settings, UserSettings user)
        {
            Settings = settings;
            User = user;
            McpAllowWrite = settings.Mcp.AllowWrite;
            Versions = Enum.GetValues<ClientVersion>()
                           .Select(v => new VersionOption { Version = v, Enabled = settings.IsEnabled(v) })
                           .ToList();

            MapView = CollectionViewSource.GetDefaultView(Maps);
            MapView.Filter = FilterMap;

            OpenMapCommand = new RelayCommand<MapEntry>(entry => _ = OpenMap(entry ?? SelectedMap));
            SaveCommand = new RelayCommand(_ => Save());
            // Cell selections are not part of the history, so a moved selection would point at stale cells after replay.
            UndoCommand = new RelayCommand(_ =>
            {
                Document?.Undo();
                ChangeSelection(Array.Empty<(int, int)>(), SelectMode.Replace);
            });
            RedoCommand = new RelayCommand(_ =>
            {
                Document?.Redo();
                ChangeSelection(Array.Empty<(int, int)>(), SelectMode.Replace);
            });
            ToolCommand = new RelayCommand<string>(name => Tool = Enum.Parse<EditTool>(name));
            LayerCommand = new RelayCommand<string>(name => Layer = Enum.Parse<EditLayer>(name));
            ZoomCommand = new RelayCommand<string>(value => Zoom = value == "in" ? Math.Min(8, Zoom * 1.25) : value == "out" ? Math.Max(0.25, Zoom / 1.25) : 1.0);
            VersionCommand = new RelayCommand<string>(name => SelectedVersion = Versions.First(v => v.Version.ToString() == name));
            DeleteSelectionCommand = new RelayCommand(_ => DeleteSelection());
            ToggleBlockSelectionCommand = new RelayCommand(_ => ToggleBlockSelection());
            CopyCommand = new RelayCommand(_ => Copy());
            PasteCommand = new RelayCommand(_ => Paste());
            SelectAllCommand = new RelayCommand(_ =>
            {
                if (Document != null)
                    SelectRect(0, 0, Document.Width - 1, Document.Height - 1, SelectMode.Replace);
            });
            ClearSelectionCommand = new RelayCommand(_ => ClearSelection());
            ToggleDoorCommand = new RelayCommand(_ => ToggleDoor(SelectedMapDoor));
            DeleteDoorCommand = new RelayCommand(_ => DeleteDoor(SelectedMapDoor));
            EditDoorCommand = new RelayCommand(_ => EditDoor());
            AddDoorPairCommand = new RelayCommand(_ => AddDoorPair());
            RemoveDoorPairCommand = new RelayCommand(_ =>
            {
                if (SelectedDoorPair != null && DoorTable.Pairs.Remove(SelectedDoorPair))
                    DoorDefinitionsChanged();
            });
            AddDoorModelCommand = new RelayCommand(_ => AddDoorModel());
            RemoveDoorModelCommand = new RelayCommand(_ =>
            {
                if (SelectedDoorModel != null && DoorTable.Doors.Remove(SelectedDoorModel))
                    DoorDefinitionsChanged();
            });
            SaveDoorTableCommand = new RelayCommand(_ => SaveDoorTable());
            AddSpawnCommand = new RelayCommand<string>(AddSpawn);
            RemoveSpawnCommand = new RelayCommand<string>(RemoveSpawn);
            PickNpcCommand = new RelayCommand(_ =>
            {
                var entry = SelectedNpc == null ? null : PickerWindow.Show("NPC 선택", NpcChoices, Assets);
                if (entry != null)
                    SelectedNpc.Npc = entry.Id;
            });
            PickMobCommand = new RelayCommand(_ =>
            {
                var entry = SelectedMob == null ? null : PickerWindow.Show("몹 선택", MobChoices, Assets);
                if (entry != null)
                    SelectedMob.Mob = entry.Id;
            });
            PickWarpMapCommand = new RelayCommand(_ =>
            {
                var entry = SelectedWarp == null ? null : PickerWindow.Show("목적지 맵 선택", MapChoices);
                if (entry != null)
                    SelectedWarp.DestMap = entry.Id;
            });
            NpcDirectionCommand = new RelayCommand<string>(direction =>
            {
                if (SelectedNpc != null)
                    SelectedNpc.Direction = direction;
            });
            OpenWarpDestinationCommand = new RelayCommand(_ => _ = OpenWarpDestination(SelectedWarp));
            DeleteEntityCommand = new RelayCommand(_ => RemoveEntity(SelectedEntity));
            FocusEntityCommand = new RelayCommand(_ => FocusEntity(SelectedEntity));
            ValidateCommand = new RelayCommand(_ =>
            {
                Validate();
                IssuesRequested?.Invoke(IssueTab.Validation);
            });
            FocusValidationCommand = new RelayCommand<ValidationItem>(item =>
            {
                if (item != null)
                    Jump(item.X, item.Y);
            });
            FocusDoorCommand = new RelayCommand<MapDoor>(door =>
            {
                if (door != null)
                    Jump(door.X, door.Y);
            });
            ExitCommand = new RelayCommand(_ => Application.Current.MainWindow?.Close());
            NavigateBackCommand = new RelayCommand(_ => _ = Navigate(back: true));
            NavigateForwardCommand = new RelayCommand(_ => _ = Navigate(back: false));
            CheckWarpsCommand = new RelayCommand<string>(scope =>
            {
                IssuesRequested?.Invoke(IssueTab.Warps);
                _ = CheckWarps(all: scope == "all");
            });
            FocusWarpIssueCommand = new RelayCommand<WarpIssue>(issue => _ = FocusWarpIssue(issue));
            DeleteCheckedWarpsCommand = new RelayCommand(_ => DeleteCheckedWarps());
            ShortcutEditorCommand = new RelayCommand(_ => ShortcutEditorRequested?.Invoke());

            ShortcutAction Shortcut(string id, string category, string label, string defaults, Action execute)
            {
                return new ShortcutAction { Id = id, Category = category, Label = label, Default = defaults, Execute = execute };
            }
            Shortcuts = new List<ShortcutAction>
            {
                Shortcut("Save", "파일", "저장", "Ctrl+S", Save),
                Shortcut("Undo", "편집", "실행 취소", "Ctrl+Z", () => UndoCommand.Execute(null)),
                Shortcut("Redo", "편집", "다시 실행", "Ctrl+Y, Ctrl+Shift+Z", () => RedoCommand.Execute(null)),
                Shortcut("Copy", "편집", "복사", "Ctrl+C", Copy),
                Shortcut("Paste", "편집", "붙여넣기", "Ctrl+V", () => PasteCommand.Execute(null)),
                Shortcut("Delete", "편집", "삭제 (선택한 스폰 + 현재 레이어)", "Delete", DeleteSelection),
                Shortcut("ToggleBlock", "편집", "블록 토글", "Ctrl+B", ToggleBlockSelection),
                Shortcut("SelectAll", "편집", "전체 선택", "Ctrl+A", () => SelectAllCommand.Execute(null)),
                Shortcut("ClearSelection", "편집", "선택 해제", "Escape", ClearSelection),
                Shortcut("EditDoor", "편집", "문 편집 (한 행 선택)", "Ctrl+D", EditDoor),
                Shortcut("NavigateBack", "이동", "뒤로 (이전에 보던 위치)", "Alt+Left, XButton1", () => NavigateBackCommand.Execute(null)),
                Shortcut("NavigateForward", "이동", "앞으로", "Alt+Right, XButton2", () => NavigateForwardCommand.Execute(null)),
                Shortcut("FindMap", "이동", "맵 검색", "Ctrl+P", () => MapSearchRequested?.Invoke()),
                Shortcut("OpenWarpDestination", "이동", "선택한 워프의 목적지 열기", "F12", () => _ = OpenWarpDestination(SelectedWarp)),
                Shortcut("ZoomIn", "보기", "확대", "Ctrl+OemPlus, Ctrl+Add", () => ZoomCommand.Execute("in")),
                Shortcut("ZoomOut", "보기", "축소", "Ctrl+OemMinus, Ctrl+Subtract", () => ZoomCommand.Execute("out")),
                Shortcut("ZoomReset", "보기", "100%", "Ctrl+0", () => ZoomCommand.Execute("reset")),
                Shortcut("ToggleGrid", "보기", "그리드 켜기/끄기", "Ctrl+G", () => ShowGrid = !ShowGrid),
                Shortcut("ToggleBlocks", "보기", "블록 표시 켜기/끄기", "Ctrl+Shift+B", () => ShowBlocks = !ShowBlocks),
                Shortcut("ToggleCollision", "보기", "유효 충돌 표시 켜기/끄기", "Ctrl+Shift+C", () => ShowCollision = !ShowCollision),
                Shortcut("ToggleSprites", "보기", "NPC / 몹 그림 켜기/끄기", "Ctrl+Shift+N", () => ShowSprites = !ShowSprites),
                Shortcut("ToggleMinimap", "보기", "미니맵 켜기/끄기", "Ctrl+M", () => ShowMinimap = !ShowMinimap),
                Shortcut("ToolSelect", "도구", "선택 / 이동", "V", () => Tool = EditTool.Select),
                Shortcut("ToolBrush", "도구", "브러시", "B", () => Tool = EditTool.Brush),
                Shortcut("ToolRect", "도구", "사각형 채우기", "R", () => Tool = EditTool.Rect),
                Shortcut("ToolFill", "도구", "영역 채우기", "G", () => Tool = EditTool.Fill),
                Shortcut("ToolEraser", "도구", "지우개", "E", () => Tool = EditTool.Eraser),
                Shortcut("ToolEyedropper", "도구", "스포이트", "I", () => Tool = EditTool.Eyedropper),
                Shortcut("ToolDoor", "도구", "문 배치", "D", () => Tool = EditTool.Door),
                Shortcut("ToolMob", "도구", "몹 영역 그리기", "M", () => Tool = EditTool.Mob),
                Shortcut("LayerTile", "도구", "편집 레이어: 타일", "1", () => Layer = EditLayer.Tile),
                Shortcut("LayerObject", "도구", "편집 레이어: 오브젝트", "2", () => Layer = EditLayer.Object),
                Shortcut("LayerBlock", "도구", "편집 레이어: 블록", "3", () => Layer = EditLayer.Block),
                Shortcut("Validate", "검사", "검증 실행", "F5", () => ValidateCommand.Execute(null)),
                Shortcut("CheckWarps", "검사", "현재 맵 워프 검사", "F6", () => CheckWarpsCommand.Execute("map")),
                Shortcut("CheckAllWarps", "검사", "전체 맵 워프 검사", "Shift+F6", () => CheckWarpsCommand.Execute("all")),
                Shortcut("ShortcutEditor", "설정", "단축키 설정", "Ctrl+OemComma", () => ShortcutEditorRequested?.Invoke()),
            };
            foreach (var shortcut in Shortcuts)
                shortcut.Text = user.Shortcuts.TryGetValue(shortcut.Id, out var text) ? text : shortcut.Default;
            ShortcutMap = Shortcuts.ToDictionary(s => s.Id);
        }

        /// <summary>
        /// Runs the action bound to the gesture. typing: a text box has focus, so gestures that would type are left to it.
        /// </summary>
        public bool RunShortcut(Gesture gesture, bool typing)
        {
            if (typing && gesture.TypesText)
                return false;

            var shortcut = Shortcuts.FirstOrDefault(s => s.Gestures.Contains(gesture));
            if (shortcut == null)
                return false;

            shortcut.Execute();
            return true;
        }

        /// <summary>
        /// Stores every shortcut in the user settings file.
        /// </summary>
        public void SaveShortcuts()
        {
            User.Shortcuts = Shortcuts.ToDictionary(s => s.Id, s => s.Text);
            try
            {
                User.Save();
                StatusText = $"단축키 저장: {UserSettings.FilePath}";
            }
            catch (Exception e)
            {
                StatusText = $"단축키 저장 실패: {e.Message}";
            }
        }

        private NavLocation? Here => Document == null ? null : new NavLocation(Document.Id, ViewCenter.X, ViewCenter.Y);

        /// <summary>
        /// Pushes the current view onto the back history before a jump; a new jump drops the forward history.
        /// </summary>
        private void RecordLocation()
        {
            if (Here is not NavLocation here)
                return;

            var last = _back.Count > 0 ? _back[^1] : (NavLocation?)null;
            if (last is not NavLocation previous || previous.Map != here.Map || Math.Abs(previous.X - here.X) >= 3 || Math.Abs(previous.Y - here.Y) >= 3)
            {
                _back.Add(here);
                if (_back.Count > 100)
                    _back.RemoveAt(0);
            }
            _forward.Clear();
            OnPropertyChanged(nameof(CanNavigateBack));
            OnPropertyChanged(nameof(CanNavigateForward));
        }

        /// <summary>
        /// Centers a cell of the open map and remembers where the view was.
        /// </summary>
        public void Jump(int x, int y)
        {
            RecordLocation();
            FocusRequested?.Invoke(x, y);
        }

        public async Task Navigate(bool back)
        {
            var from = back ? _back : _forward;
            var to = back ? _forward : _back;
            if (Busy || from.Count == 0 || Here is not NavLocation here)
                return;

            var target = from[^1];
            from.RemoveAt(from.Count - 1);
            if (target.Map != here.Map)
            {
                var entry = Maps.FirstOrDefault(m => m.Id == target.Map);
                if (entry != null)
                    await OpenMap(entry, record: false);

                // Cancelled at the save prompt or the file is gone: keep the entry for another try.
                if (Document?.Id != target.Map)
                {
                    if (entry != null)
                        from.Add(target);
                    OnPropertyChanged(nameof(CanNavigateBack));
                    OnPropertyChanged(nameof(CanNavigateForward));
                    return;
                }
                SelectedMap = entry;
            }
            to.Add(here);
            CenterRequested?.Invoke(target.X, target.Y);
            OnPropertyChanged(nameof(CanNavigateBack));
            OnPropertyChanged(nameof(CanNavigateForward));
        }

        private void BeginLoading(string title)
        {
            var id = ++_loadingId;
            LoadingSteps.Clear();
            LoadingTitle = title;
            LoadingDone = 0;
            LoadingTotal = 0;
            Task.Delay(300).ContinueWith(_ =>
            {
                if (_loadingId == id)
                    LoadingVisible = true;
            }, TaskScheduler.FromCurrentSynchronizationContext());
        }

        private void EndLoading()
        {
            _loadingId++;
            LoadingVisible = false;
        }

        /// <summary>
        /// Runs work on the thread pool as one line of the loading popup. Steps started together run in parallel.
        /// </summary>
        private async Task<T> LoadStep<T>(string name, Func<T> work)
        {
            var step = new LoadingStep { Name = name };
            LoadingSteps.Add(step);
            LoadingTotal = LoadingSteps.Count;
            var watch = System.Diagnostics.Stopwatch.StartNew();
            try
            {
                var result = await Task.Run(work);
                step.Milliseconds = watch.ElapsedMilliseconds;
                step.Done = true;
                return result;
            }
            catch
            {
                step.Failed = true;
                throw;
            }
            finally
            {
                LoadingDone = LoadingSteps.Count(s => s.Done || s.Failed);
            }
        }

        /// <summary>
        /// Reads the tables and the first editable client's resources at the same time. Opening map.xlsx takes most
        /// of the time, so the other workbooks and the resources finish while it is still being read.
        /// </summary>
        public async Task Initialize()
        {
            Busy = true;
            StatusText = "데이터 읽는 중...";
            BeginLoading("데이터 불러오는 중");

            var settings = Settings;
            var tables = settings.TableDirectory;
            var assetTasks = new List<(ClientVersion Version, Task<ClientAssets> Task)>();
            var initial = Versions.FirstOrDefault(v => v.Enabled)?.Version;
            if (initial is ClientVersion version)
            {
                assetTasks.Add((version, LoadStep($"{version} 리소스", () => ClientAssets.Load(version, settings.ClientDirectory(version)))));
                if (version != ClientVersion.v550 && ShowMissing550 && settings.IsEnabled(ClientVersion.v550))
                    assetTasks.Add((ClientVersion.v550, LoadStep("v550 리소스 (5.50에 없는 타일 표시용)", () => ClientAssets.Load(ClientVersion.v550, settings.ClientDirectory(ClientVersion.v550)))));
            }

            try
            {
                var npcTask = LoadStep(SpawnTable.NpcFile, () => XlsxFile.Open(Path.Combine(tables, SpawnTable.NpcFile)));
                var mobTask = LoadStep(SpawnTable.MobFile, () => XlsxFile.Open(Path.Combine(tables, SpawnTable.MobFile)));
                var mapTask = LoadStep(SpawnTable.MapFile, () => XlsxFile.Open(Path.Combine(tables, SpawnTable.MapFile)));
                var doorTask = LoadStep("door.xlsx", () => DoorTable.Read(Path.Combine(tables, "door.xlsx")));
                var idsTask = LoadStep("맵 목록", () => Directory.Exists(settings.MapDirectory)
                    ? Directory.EnumerateFiles(settings.MapDirectory, "*.map")
                               .Select(p => int.TryParse(Path.GetFileNameWithoutExtension(p), out var id) ? id : -1)
                               .Where(id => id >= 0)
                               .OrderBy(id => id)
                               .ToList()
                    : new List<int>());
                await Task.WhenAll(npcTask, mobTask, mapTask, doorTask, idsTask);

                var spawns = await LoadStep("이름·스폰 시트 정리", () => new SpawnTable(tables, npcTask.Result, mobTask.Result, mapTask.Result));
                var doors = doorTask.Result;
                var ids = idsTask.Result;

                _spawns = spawns;
                OnPropertyChanged(nameof(NpcChoices));
                OnPropertyChanged(nameof(MobChoices));
                OnPropertyChanged(nameof(MapChoices));
                DoorTable = doors;
                foreach (var pair in DoorTable.Pairs)
                    pair.PropertyChanged += (s, e) => DoorDefinitionsChanged();
                foreach (var model in DoorTable.Doors)
                    model.PropertyChanged += (s, e) => DoorDefinitionsChanged();
                DoorModelsView = new ListCollectionView(DoorTable.Doors)
                {
                    Filter = item => OnlyMapDoors == false || (Document?.Doors.Any(d => d.Model == item) ?? false),
                };

                foreach (var id in ids)
                    Maps.Add(new MapEntry { Id = id, Name = _spawns.MapNames.Find(id) });

                StatusText = $"맵 {Maps.Count}개";
            }
            catch (Exception e)
            {
                StatusText = $"초기화 실패: {e.Message}";
            }

            foreach (var (assetVersion, task) in assetTasks)
            {
                try
                {
                    _assetCache[assetVersion] = await task;
                }
                catch (Exception e)
                {
                    StatusText = $"{assetVersion} 리소스 로드 실패: {e.Message}";
                }
            }
            EndLoading();
            Busy = false;

            SelectedVersion = Versions.FirstOrDefault(v => v.Enabled);
            if (SelectedVersion == null)
                StatusText = "Client.v550 / Client.v651 경로가 비어 있거나 TILE.DAT가 없어 편집할 수 없습니다. MAPEDITOR_ENVIRONMENT 환경변수와 appsettings.{환경}.json(예: appsettings.Local.json)을 확인하세요.";
        }

        private bool FilterMap(object item)
        {
            if (string.IsNullOrWhiteSpace(MapQuery))
                return true;

            var entry = (MapEntry)item;
            return entry.Label.Contains(MapQuery.Trim(), StringComparison.OrdinalIgnoreCase);
        }

        private void OnMapQueryChanged()
        {
            MapView.Refresh();
        }

        private async void OnSelectedVersionChanged()
        {
            if (SelectedVersion == null)
                return;

            if (SelectedVersion.Enabled == false)
            {
                StatusText = $"{SelectedVersion.Version} 경로가 설정되지 않아 사용할 수 없습니다.";
                SelectedVersion = Versions.FirstOrDefault(v => v.Version == Assets?.Version);
                return;
            }

            var version = SelectedVersion.Version;
            if (_assetCache.TryGetValue(version, out var cached) == false)
            {
                Busy = true;
                StatusText = $"{version} 리소스 읽는 중...";
                BeginLoading($"{version} 리소스 불러오는 중");
                try
                {
                    var directory = Settings.ClientDirectory(version);
                    cached = await LoadStep($"{version} 리소스", () => ClientAssets.Load(version, directory));
                    _assetCache[version] = cached;
                }
                catch (Exception e)
                {
                    StatusText = $"{version} 리소스 로드 실패: {e.Message}";
                    return;
                }
                finally
                {
                    EndLoading();
                    Busy = false;
                }
            }

            Assets = cached;
            StatusText = $"{version} 리소스: 타일 {Assets.TileCount}, 오브젝트 {Assets.Objects.Count}";
            await UpdateLimit550();
            RenderInvalidated?.Invoke();
        }

        /// <summary>
        /// Loads the 5.50 resources once (for their counts) when ShowMissing550 applies to the selected version.
        /// </summary>
        private async Task UpdateLimit550()
        {
            if (ShowMissing550 == false || Assets == null || Assets.Version == ClientVersion.v550 || Settings.IsEnabled(ClientVersion.v550) == false)
            {
                Limit550 = null;
                return;
            }

            if (_assetCache.TryGetValue(ClientVersion.v550, out var assets550) == false)
            {
                var directory = Settings.ClientDirectory(ClientVersion.v550);
                BeginLoading("v550 리소스 불러오는 중");
                try
                {
                    assets550 = await LoadStep("v550 리소스 (5.50에 없는 타일 표시용)", () => ClientAssets.Load(ClientVersion.v550, directory));
                    _assetCache[ClientVersion.v550] = assets550;
                }
                catch (Exception e)
                {
                    StatusText = $"5.50 리소스 로드 실패 (5.50에 없는 타일 표시 끔): {e.Message}";
                    Limit550 = null;
                    return;
                }
                finally
                {
                    EndLoading();
                }
            }
            Limit550 = (assets550.TileCount, assets550.Objects.Count);
        }

        private async void OnShowMissing550Changed()
        {
            await UpdateLimit550();
            RenderInvalidated?.Invoke();
        }

        private void OnOnlyMapDoorsChanged() => RefreshDoorModels();

        /// <summary>
        /// Called whenever Document.Doors is recomputed.
        /// </summary>
        private void RefreshDoorModels()
        {
            OnPropertyChanged(nameof(SelectedDoorUsages));
            if (DoorModelsView == null || DoorModelsView.IsEditingItem || DoorModelsView.IsAddingNew)
                return;

            DoorModelsView.Refresh();
        }

        private void OnShowTilesChanged() => RenderInvalidated?.Invoke();
        private void OnShowObjectsChanged() => RenderInvalidated?.Invoke();
        private void OnShowBlocksChanged() => OverlayInvalidated?.Invoke();
        private void OnShowCollisionChanged() => OverlayInvalidated?.Invoke();
        private void OnShowGridChanged() => OverlayInvalidated?.Invoke();
        private void OnShowDoorsChanged() => OverlayInvalidated?.Invoke();
        private void OnShowNpcsChanged() => OverlayInvalidated?.Invoke();
        private void OnShowMobsChanged() => OverlayInvalidated?.Invoke();
        private void OnShowWarpsChanged() => OverlayInvalidated?.Invoke();
        private void OnShowSpritesChanged() => OverlayInvalidated?.Invoke();
        private void OnSelectedMapDoorChanged() => OverlayInvalidated?.Invoke();

        // Picking a row in a spawn grid makes it the single selected entity.
        private void OnSelectedNpcChanged() => SelectFromGrid(SelectedNpc);
        private void OnSelectedMobChanged() => SelectFromGrid(SelectedMob);
        private void OnSelectedWarpChanged() => SelectFromGrid(SelectedWarp);

        private void SelectFromGrid(Entity entity)
        {
            if (entity != null && SelectedEntities.Contains(entity) == false)
            {
                SelectedEntities.Clear();
                SelectedEntities.Add(entity);
                UpdateSelectionText();
            }
            if (entity != null)
                SelectedEntity = entity;
            OverlayInvalidated?.Invoke();
        }

        /// <summary>
        /// record: remember the current view for back navigation (false while navigating the history).
        /// </summary>
        public async Task OpenMap(MapEntry entry, bool record = true)
        {
            if (entry == null || CanOpen == false)
                return;

            if (Document != null && Document.Dirty)
            {
                var answer = MessageBox.Show($"{Document.Title} 변경 사항을 저장할까요?", "맵 에디터", MessageBoxButton.YesNoCancel);
                if (answer == MessageBoxResult.Cancel)
                    return;
                else if (answer == MessageBoxResult.Yes)
                    Save();
            }
            if (record && Document != null && Document.Id != entry.Id)
                RecordLocation();

            Busy = true;
            StatusText = $"{entry.Label} 여는 중...";
            try
            {
                var directory = Settings.MapDirectory;
                var spawns = _spawns;
                var (document, npcs, mobs, warps) = await Task.Run(() =>
                    (MapDocument.Open(directory, entry.Id, entry.Name), spawns.ReadNpc(entry.Id), spawns.ReadMob(entry.Id), spawns.ReadWarp(entry.Id)));

                if (Document != null)
                    Document.CellsChanged -= OnCellsChanged;

                document.Attach(npcs, mobs, warps);
                Selection.Clear();
                SelectedEntities.Clear();
                SelectedEntity = null;
                SelectedNpc = null;
                SelectedMob = null;
                SelectedWarp = null;
                Document = document;
                Document.CellsChanged += OnCellsChanged;
                WatchSpawns(Document.Npcs);
                WatchSpawns(Document.Mobs);
                WatchSpawns(Document.Warps);
                Document.Doors = DoorTable.Find(Document.Width, Document.Height, Document.Map.Objects);
                RefreshDoorModels();
                Validation.Clear();
                UpdateSelectionText();
                StatusText = $"{entry.Label} {Document.Width}x{Document.Height}, 블록 {Document.Blocks.Count}, 문 {Document.Doors.Count}, NPC {Document.Npcs.Count}, 몹 {Document.Mobs.Count}, 워프 {Document.Warps.Count}";
            }
            catch (Exception e)
            {
                StatusText = $"맵 열기 실패: {e.Message}";
            }
            finally
            {
                Busy = false;
            }
            RenderInvalidated?.Invoke();
        }

        /// <summary>
        /// Keeps names in step with ids, drops removed entities from the selection and redraws overlays.
        /// </summary>
        private void WatchSpawns<T>(ObservableCollection<T> items) where T : Entity
        {
            void OnChanged(object sender, PropertyChangedEventArgs e)
            {
                if (sender is NpcSpawn npc && e.PropertyName == nameof(NpcSpawn.Npc))
                    npc.Info = _spawns.NpcNames.Entry(npc.Npc);
                else if (sender is MobSpawn mob && e.PropertyName == nameof(MobSpawn.Mob))
                    mob.Info = _spawns.MobNames.Entry(mob.Mob);
                else if (sender is WarpEntry warp && e.PropertyName == nameof(WarpEntry.Dest))
                    warp.DestName = warp.DestMap is int id ? _spawns.MapNames.Find(id) : "";
                OverlayInvalidated?.Invoke();
                OnPropertyChanged(nameof(Document));
            }

            foreach (var item in items)
                item.PropertyChanged += OnChanged;

            items.CollectionChanged += (s, e) =>
            {
                if (e.NewItems != null)
                {
                    foreach (T item in e.NewItems)
                        item.PropertyChanged += OnChanged;
                }
                if (e.OldItems != null)
                {
                    foreach (T item in e.OldItems)
                    {
                        item.PropertyChanged -= OnChanged;
                        SelectedEntities.Remove(item);
                        if (ReferenceEquals(SelectedEntity, item))
                            SelectedEntity = null;
                    }
                }
                UpdateSelectionText();
                OverlayInvalidated?.Invoke();
                OnPropertyChanged(nameof(Document));
            };
        }

        [PropertyChanged.SuppressPropertyChangedWarnings]
        private void OnCellsChanged(IReadOnlyList<(int X, int Y)> cells)
        {
            Document.Doors = DoorTable.Find(Document.Width, Document.Height, Document.Map.Objects);
            RefreshDoorModels();
            SelectedMapDoor = null;
            UpdateSelectionText();
            OnPropertyChanged(nameof(Document));
        }

        private void OnPropertyChanged(string name)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }

        public void Save()
        {
            if (Document == null)
                return;

            try
            {
                var saved = new List<string>();
                if (Document.MapDirty)
                {
                    Document.SaveMap(Settings.MapDirectory);
                    saved.Add(".map/.block");
                }
                if (Document.NpcDirty)
                {
                    _spawns.SaveNpc(Document.Id, Document.Npcs);
                    Document.NpcDirty = false;
                    saved.Add("npc_spawn");
                }
                if (Document.MobDirty)
                {
                    _spawns.SaveMob(Document.Id, Document.Mobs);
                    Document.MobDirty = false;
                    saved.Add("mob_spawn");
                }
                if (Document.WarpDirty)
                {
                    _spawns.SaveWarp(Document.Id, Document.Warps);
                    Document.WarpDirty = false;
                    saved.Add("warp");
                }
                StatusText = saved.Count == 0 ? "변경 사항 없음" : $"저장: {string.Join(", ", saved)}";
            }
            catch (IOException e)
            {
                StatusText = $"저장 실패 (엑셀에서 파일을 열고 있는지 확인): {e.Message}";
            }
            catch (Exception e)
            {
                StatusText = $"저장 실패: {e.Message}";
            }
            OnPropertyChanged(nameof(Document));
        }

        /// <summary>
        /// Server rule (map::blocked) with the SObj table of the selected client.
        /// </summary>
        public bool IsBlocked(int x, int y)
        {
            if (Document == null || Document.Map.Contains(x, y) == false)
                return true;

            var cell = Document.Get(x, y);
            if (cell.Block || cell.Tile == 0)
                return true;

            var sobj = Assets?.Objects.Find(cell.Object);
            return sobj != null && (sobj.Collision & 0x0F) == 0x0F;
        }

        public void Hover(int x, int y)
        {
            if (Document == null || Document.Map.Contains(x, y) == false)
            {
                _hoverCell = null;
                HoverText = "";
                return;
            }
            _hoverCell = (x, y);

            var cell = Document.Get(x, y);
            var sobj = Assets?.Objects.Find(cell.Object);
            var objectText = sobj == null ? $"{cell.Object}" : $"{cell.Object} (h{sobj.Frames.Length}, col 0x{sobj.Collision:X})";
            var text = $"X {x}, Y {y} | 타일 {cell.Tile} | 오브젝트 {objectText} | 블록 {(cell.Block ? "O" : "X")} | {(IsBlocked(x, y) ? "막힘" : "이동 가능")}";
            if (IsBlocked(x, y) == false && sobj != null && (sobj.Collision & 0x0F) != 0)
            {
                var sides = new[] { (2, "위"), (8, "오른쪽"), (1, "아래"), (4, "왼쪽") }.Where(s => (sobj.Collision & s.Item1) != 0).Select(s => s.Item2);
                text += $" (막힌 변: {string.Join(", ", sides)})";
            }
            if (Limit550 is (int tiles, int objects) && (cell.Tile >= tiles || cell.Object > objects))
                text += " | 5.50에 없음";

            var door = DoorAt(x, y);
            if (door != null)
                text += $" | 문 {door.Model.Id} {(door.Opened ? "열림" : "닫힘")}";

            var npc = NpcAt(x, y);
            if (npc != null)
                text += $" | NPC {npc.Npc} {npc.Name} {npc.Direction}";

            var warp = WarpAt(x, y);
            if (warp != null)
                text += $" | 워프 → {warp.Dest} {warp.DestName}";

            var mobs = Document.Mobs.Count(m => m.Contains(x, y));
            if (mobs > 0)
                text += $" | 몹 스폰 {mobs}개";

            HoverText = text;
        }

        public NpcSpawn NpcAt(int x, int y) => Document?.Npcs.LastOrDefault(n => n.X == x && n.Y == y);
        public WarpEntry WarpAt(int x, int y) => Document?.Warps.LastOrDefault(w => w.X == x && w.Y == y);
        public MapDoor DoorAt(int x, int y) => Document?.Doors.FirstOrDefault(d => d.Y == y && x >= d.X && x < d.X + d.Width);

        /// <summary>
        /// The base cell of the object drawn at map point (mapX, mapY) in cells: the cell itself, or a cell below
        /// whose stack reaches up to it. Only opaque pixels count, so clicking the empty part of a tall object picks
        /// the cell under the cursor. Lower cells are drawn later, so they win.
        /// </summary>
        public (int X, int Y)? ObjectCellAt(double mapX, double mapY)
        {
            if (Document == null || Assets == null)
                return null;

            var x = (int)Math.Floor(mapX);
            var y = (int)Math.Floor(mapY);
            var px = (int)((mapX - x) * Assets.CellPixels);
            var py = (int)((mapY - y) * Assets.CellPixels);
            for (int k = Math.Max(1, Assets.Objects.MaxHeight) - 1; k >= 0; k--)
            {
                var cy = y + k;
                if (Document.Map.Contains(x, cy) == false)
                    continue;

                var sobj = Assets.Objects.Find(Document.Get(x, cy).Object);
                if (sobj == null || k >= sobj.Frames.Length)
                    continue;

                var frame = Assets.ObjectFrame(sobj.Frames[k]);
                if (frame == null)
                    continue;

                var lx = px - frame.Left;
                var ly = py - frame.Top;
                if (lx >= 0 && ly >= 0 && lx < frame.Width && ly < frame.Height && frame.Pixels[ly * frame.Width + lx] != 0)
                    return (x, cy);
            }
            return null;
        }

        private CellValue WithLayerValue(CellValue cell, bool erase)
        {
            if (Layer == EditLayer.Tile)
                cell.Tile = erase ? (ushort)0 : (ushort)SelectedTile;
            else if (Layer == EditLayer.Object)
                cell.Object = erase ? (ushort)0 : (ushort)SelectedObject;
            else
                cell.Block = erase == false;
            return cell;
        }

        private int LayerValue(CellValue cell)
        {
            if (Layer == EditLayer.Tile)
                return cell.Tile;
            else if (Layer == EditLayer.Object)
                return cell.Object;
            else
                return cell.Block ? 1 : 0;
        }

        /// <summary>
        /// Brush or eraser on cells of the active layer. merge joins the previous undo step (same stroke).
        /// </summary>
        public void Paint(IEnumerable<(int X, int Y)> cells, bool erase, bool merge)
        {
            if (Document == null)
                return;

            Document.Apply(cells.Where(c => Document.Map.Contains(c.X, c.Y))
                                .Select(c => (c.X, c.Y, WithLayerValue(Document.Get(c.X, c.Y), erase))), merge);
        }

        /// <summary>
        /// One undo step that rewrites the given cells; used by the context menu actions.
        /// </summary>
        public void EditCells(IEnumerable<(int X, int Y)> cells, Func<CellValue, CellValue> edit)
        {
            if (Document == null)
                return;

            Document.Apply(cells.Where(c => Document.Map.Contains(c.X, c.Y))
                                .Select(c => (c.X, c.Y, edit(Document.Get(c.X, c.Y)))).ToList());
        }

        public void FillRect(int x0, int y0, int x1, int y1)
        {
            Paint(Rect(x0, y0, x1, y1), erase: false, merge: false);
        }

        public void FloodFill(int x, int y)
        {
            if (Document == null || Document.Map.Contains(x, y) == false)
                return;

            var target = LayerValue(Document.Get(x, y));
            var cells = new List<(int X, int Y)>();
            var visited = new HashSet<(int, int)> { (x, y) };
            var queue = new Queue<(int X, int Y)>();
            queue.Enqueue((x, y));
            while (queue.Count > 0 && cells.Count < 200_000)
            {
                var (cx, cy) = queue.Dequeue();
                cells.Add((cx, cy));
                foreach (var (nx, ny) in new[] { (cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1) })
                {
                    if (Document.Map.Contains(nx, ny) && visited.Add((nx, ny)) && LayerValue(Document.Get(nx, ny)) == target)
                        queue.Enqueue((nx, ny));
                }
            }
            Paint(cells, erase: false, merge: false);
        }

        public void Pick(int x, int y)
        {
            if (Document == null || Document.Map.Contains(x, y) == false)
                return;

            var cell = Document.Get(x, y);
            if (Layer == EditLayer.Tile)
                SelectedTile = cell.Tile;
            else if (Layer == EditLayer.Object)
                SelectedObject = cell.Object;
            Tool = EditTool.Brush;
        }

        public void PickTile(int x, int y)
        {
            SelectedTile = Document.Get(x, y).Tile;
            Layer = EditLayer.Tile;
            Tool = EditTool.Brush;
        }

        public void PickObject(int x, int y)
        {
            SelectedObject = Document.Get(x, y).Object;
            Layer = EditLayer.Object;
            Tool = EditTool.Brush;
        }

        public static IEnumerable<(int X, int Y)> Rect(int x0, int y0, int x1, int y1)
        {
            for (int y = Math.Min(y0, y1); y <= Math.Max(y0, y1); y++)
            {
                for (int x = Math.Min(x0, x1); x <= Math.Max(x0, x1); x++)
                    yield return (x, y);
            }
        }

        public void ChangeSelection(IEnumerable<(int X, int Y)> cells, SelectMode mode)
        {
            _floatingPaste = null;
            if (mode == SelectMode.Replace)
                Selection.Clear();

            foreach (var cell in cells)
            {
                if (Document == null || Document.Map.Contains(cell.X, cell.Y) == false)
                    continue;

                if (mode == SelectMode.Remove)
                    Selection.Remove(cell);
                else
                    Selection.Add(cell);
            }
            UpdateSelectionText();
            OverlayInvalidated?.Invoke();
        }

        /// <summary>
        /// Rubber band: the cells in the rectangle plus every NPC, warp and mob area that lies fully inside it.
        /// </summary>
        public void SelectRect(int x0, int y0, int x1, int y1, SelectMode mode)
        {
            if (Document == null)
                return;

            var left = Math.Min(x0, x1);
            var top = Math.Min(y0, y1);
            var right = Math.Max(x0, x1);
            var bottom = Math.Max(y0, y1);
            bool Inside(int x, int y) => x >= left && x <= right && y >= top && y <= bottom;

            var entities = new List<Entity>();
            entities.AddRange(Document.Npcs.Where(n => Inside(n.X, n.Y)));
            entities.AddRange(Document.Warps.Where(w => Inside(w.X, w.Y)));
            entities.AddRange(Document.Mobs.Where(m => Inside(m.Left, m.Top) && Inside(m.Right, m.Bottom)));

            if (mode == SelectMode.Replace)
                SelectedEntities.Clear();
            foreach (var entity in entities)
            {
                if (mode == SelectMode.Remove)
                    SelectedEntities.Remove(entity);
                else
                    SelectedEntities.Add(entity);
            }
            SelectedEntity = SelectedEntities.Count == 1 ? SelectedEntities.First() : null;
            ChangeSelection(Rect(left, top, right, bottom), mode);
        }

        /// <summary>
        /// Click on an NPC, warp or mob area. add keeps the current selection (Shift).
        /// </summary>
        public void SelectEntity(Entity entity, bool add)
        {
            if (add == false)
            {
                SelectedEntities.Clear();
                Selection.Clear();
            }
            SelectedEntities.Add(entity);
            SelectedEntity = entity;
            if (entity is NpcSpawn npc)
                SelectedNpc = npc;
            else if (entity is MobSpawn mob)
                SelectedMob = mob;
            else if (entity is WarpEntry warp)
                SelectedWarp = warp;
            UpdateSelectionText();
            OverlayInvalidated?.Invoke();
        }

        /// <summary>
        /// Ctrl click on an NPC, warp or mob area: add it to the selection, or drop it when already selected.
        /// </summary>
        public void ToggleEntity(Entity entity)
        {
            if (SelectedEntities.Contains(entity))
            {
                SelectedEntities.Remove(entity);
                SelectedEntity = SelectedEntities.Count == 1 ? SelectedEntities.First() : null;
                UpdateSelectionText();
                OverlayInvalidated?.Invoke();
            }
            else
            {
                SelectEntity(entity, add: true);
            }
        }

        /// <summary>
        /// Ctrl click on a cell (an object's base cell when clicking an object): add or drop it.
        /// </summary>
        public void ToggleCell((int X, int Y) cell)
        {
            ChangeSelection(new[] { cell }, Selection.Contains(cell) ? SelectMode.Remove : SelectMode.Add);
        }

        public void ClearSelection()
        {
            SelectedEntities.Clear();
            SelectedEntity = null;
            ChangeSelection(Array.Empty<(int, int)>(), SelectMode.Replace);
        }

        /// <summary>
        /// Selects every cell whose active-layer value equals the value at (x, y).
        /// </summary>
        public void SelectSame(int x, int y, SelectMode mode)
        {
            if (Document == null || Document.Map.Contains(x, y) == false)
                return;

            var target = LayerValue(Document.Get(x, y));
            var cells = new List<(int, int)>();
            for (int cy = 0; cy < Document.Height; cy++)
            {
                for (int cx = 0; cx < Document.Width; cx++)
                {
                    if (LayerValue(Document.Get(cx, cy)) == target)
                        cells.Add((cx, cy));
                }
            }
            ChangeSelection(cells, mode);
        }

        private void UpdateSelectionText()
        {
            HasSelection = Document != null && (Selection.Count > 0 || SelectedEntities.Count > 0);
            if (Document == null || (Selection.Count == 0 && SelectedEntities.Count == 0))
            {
                SelectionText = "선택 없음";
                return;
            }

            var text = "";
            if (Selection.Count > 0)
            {
                var minX = Selection.Min(c => c.X);
                var minY = Selection.Min(c => c.Y);
                var maxX = Selection.Max(c => c.X);
                var maxY = Selection.Max(c => c.Y);
                var tiles = Selection.Select(c => Document.Get(c.X, c.Y).Tile).Distinct().Take(2).ToList();
                var objects = Selection.Select(c => Document.Get(c.X, c.Y).Object).Distinct().Take(2).ToList();
                var placed = Selection.Count(c => Document.Get(c.X, c.Y).Object != 0);
                var blocks = Selection.Count(c => Document.Blocks.Contains(c.X, c.Y));
                text = $"{Selection.Count}칸 ({minX}, {minY})-({maxX}, {maxY})\n" +
                       $"타일 {(tiles.Count == 1 ? tiles[0].ToString() : "혼합")}, " +
                       $"오브젝트 {(objects.Count == 1 ? objects[0].ToString() : "혼합")} ({placed}칸), 블록 {blocks}";
            }
            if (SelectedEntities.Count > 0)
            {
                text += (text == "" ? "" : "\n") +
                        $"NPC {SelectedEntities.OfType<NpcSpawn>().Count()}, 몹 스폰 {SelectedEntities.OfType<MobSpawn>().Count()}, 워프 {SelectedEntities.OfType<WarpEntry>().Count()}";
            }
            SelectionText = text;
        }

        /// <summary>
        /// Moves the selected objects (and tiles/blocks when enabled) and the selected NPCs, warps and mob areas by
        /// (dx, dy) as one undo step. Cells moved outside the map are dropped.
        /// </summary>
        public void MoveSelection(int dx, int dy)
        {
            if (Document == null || (dx == 0 && dy == 0))
                return;

            if (FloatingPaste && _floatingPaste is (int px, int py, _, _, bool changed))
            {
                if (changed)
                    Document.Undo();
                PasteAt(px + dx, py + dy);
                return;
            }

            using (Document.Group())
            {
                if (Selection.Count > 0)
                {
                    var sources = Selection.ToDictionary(c => c, c => Document.Get(c.X, c.Y));
                    var result = new Dictionary<(int X, int Y), CellValue>();
                    foreach (var (cell, value) in sources)
                    {
                        var cleared = value;
                        cleared.Object = 0;
                        if (MoveTiles)
                            cleared.Tile = 0;
                        if (MoveBlocks)
                            cleared.Block = false;
                        result[cell] = cleared;
                    }
                    foreach (var (cell, value) in sources)
                    {
                        var target = (cell.X + dx, cell.Y + dy);
                        if (Document.Map.Contains(target.Item1, target.Item2) == false)
                            continue;

                        var moved = result.TryGetValue(target, out var pending) ? pending : Document.Get(target.Item1, target.Item2);
                        if (value.Object != 0)
                            moved.Object = value.Object;
                        if (MoveTiles)
                            moved.Tile = value.Tile;
                        if (MoveBlocks)
                            moved.Block = value.Block;
                        result[target] = moved;
                    }
                    Document.Apply(result.Select(r => (r.Key.X, r.Key.Y, r.Value)).ToList());

                    var shifted = Selection.Select(c => (c.X + dx, c.Y + dy)).Where(c => Document.Map.Contains(c.Item1, c.Item2)).ToList();
                    Selection.Clear();
                    foreach (var cell in shifted)
                        Selection.Add(cell);
                }

                foreach (var entity in SelectedEntities)
                {
                    if (entity is NpcSpawn npc)
                    {
                        npc.X = Math.Clamp(npc.X + dx, 0, Document.Width - 1);
                        npc.Y = Math.Clamp(npc.Y + dy, 0, Document.Height - 1);
                    }
                    else if (entity is WarpEntry warp)
                    {
                        warp.X = Math.Clamp(warp.X + dx, 0, Document.Width - 1);
                        warp.Y = Math.Clamp(warp.Y + dy, 0, Document.Height - 1);
                    }
                    else if (entity is MobSpawn mob)
                    {
                        var mx = Math.Clamp(dx, -mob.Left, Document.Width - 1 - mob.Right);
                        var my = Math.Clamp(dy, -mob.Top, Document.Height - 1 - mob.Bottom);
                        mob.BeginX += mx;
                        mob.EndX += mx;
                        mob.BeginY += my;
                        mob.EndY += my;
                    }
                }
            }
            UpdateSelectionText();
            OverlayInvalidated?.Invoke();
        }

        /// <summary>
        /// Sets a mob spawn area from two corners (clamped to the map) as one undo step.
        /// </summary>
        public void SetMobArea(MobSpawn mob, int x0, int y0, int x1, int y1)
        {
            if (Document == null || mob == null)
                return;

            using (Document.Group())
            {
                mob.BeginX = Math.Clamp(Math.Min(x0, x1), 0, Document.Width - 1);
                mob.BeginY = Math.Clamp(Math.Min(y0, y1), 0, Document.Height - 1);
                mob.EndX = Math.Clamp(Math.Max(x0, x1), 0, Document.Width - 1);
                mob.EndY = Math.Clamp(Math.Max(y0, y1), 0, Document.Height - 1);
            }
        }

        public void DeleteSelection()
        {
            if (Document == null)
                return;

            using (Document.Group())
            {
                foreach (var entity in SelectedEntities.ToList())
                    RemoveEntity(entity);
                if (Selection.Count > 0)
                    Paint(Selection.ToList(), erase: true, merge: false);
            }
            SelectedEntity = null;
            UpdateSelectionText();
        }

        public void ToggleBlockSelection()
        {
            if (Document == null || Selection.Count == 0)
                return;

            var block = Selection.Any(c => Document.Blocks.Contains(c.X, c.Y) == false);
            SetBlock(Selection.ToList(), block);
        }

        public void SetBlock(IEnumerable<(int X, int Y)> cells, bool block)
        {
            EditCells(cells, cell =>
            {
                cell.Block = block;
                return cell;
            });
        }

        /// <summary>
        /// Copies the selected cells (tile, object, block layers) and spawns. When more than one kind is selected
        /// the user picks which kinds to copy.
        /// </summary>
        public void Copy()
        {
            if (Document == null)
                return;

            var npcs = SelectedEntities.OfType<NpcSpawn>().ToList();
            var warps = SelectedEntities.OfType<WarpEntry>().ToList();
            var mobs = SelectedEntities.OfType<MobSpawn>().ToList();
            var objects = Selection.Count(c => Document.Get(c.X, c.Y).Object != 0);
            var blocks = Selection.Count(c => Document.Blocks.Contains(c.X, c.Y));
            var options = new List<(CopyKind Kind, string Label)>();
            if (Selection.Count > 0)
                options.Add((CopyKind.Tile, $"타일 ({Selection.Count}칸)"));
            if (objects > 0)
                options.Add((CopyKind.Object, $"오브젝트 ({objects}칸)"));
            if (blocks > 0)
                options.Add((CopyKind.Block, $"블록 ({blocks}칸)"));
            if (npcs.Count > 0)
                options.Add((CopyKind.Npc, $"NPC ({npcs.Count}개)"));
            if (warps.Count > 0)
                options.Add((CopyKind.Warp, $"워프 ({warps.Count}개)"));
            if (mobs.Count > 0)
                options.Add((CopyKind.Mob, $"몹 스폰 ({mobs.Count}개)"));

            if (options.Count == 0)
            {
                StatusText = "복사할 것이 없습니다. 칸이나 NPC·워프·몹 스폰을 선택하세요.";
                return;
            }

            var kinds = options.Count == 1 ? new HashSet<CopyKind> { options[0].Kind } : CopyWindow.Show(options);
            if (kinds == null)
                return;

            var copyCells = kinds.Contains(CopyKind.Tile) || kinds.Contains(CopyKind.Object) || kinds.Contains(CopyKind.Block);
            var points = new List<(int X, int Y)>();
            if (copyCells)
                points.AddRange(Selection);
            if (kinds.Contains(CopyKind.Npc))
                points.AddRange(npcs.Select(n => (n.X, n.Y)));
            if (kinds.Contains(CopyKind.Warp))
                points.AddRange(warps.Select(w => (w.X, w.Y)));
            if (kinds.Contains(CopyKind.Mob))
                points.AddRange(mobs.SelectMany(m => new[] { (m.Left, m.Top), (m.Right, m.Bottom) }));

            var minX = points.Min(p => p.X);
            var minY = points.Min(p => p.Y);
            Clipboard = new ClipboardContent
            {
                Width = points.Max(p => p.X) - minX + 1,
                Height = points.Max(p => p.Y) - minY + 1,
                Kinds = kinds,
                Cells = copyCells ? Selection.Select(c => (c.X - minX, c.Y - minY, Document.Get(c.X, c.Y))).ToList() : new List<(int, int, CellValue)>(),
                Npcs = kinds.Contains(CopyKind.Npc)
                    ? npcs.Select(n => new NpcSpawn { Npc = n.Npc, Info = n.Info, X = n.X - minX, Y = n.Y - minY, Direction = n.Direction }).ToList()
                    : new List<NpcSpawn>(),
                Warps = kinds.Contains(CopyKind.Warp)
                    ? warps.Select(w => new WarpEntry { X = w.X - minX, Y = w.Y - minY, Dest = w.Dest, Condition = w.Condition, DestName = w.DestName }).ToList()
                    : new List<WarpEntry>(),
                Mobs = kinds.Contains(CopyKind.Mob)
                    ? mobs.Select(m => new MobSpawn
                    {
                        Mob = m.Mob,
                        Info = m.Info,
                        BeginX = m.Left - minX,
                        BeginY = m.Top - minY,
                        EndX = m.Right - minX,
                        EndY = m.Bottom - minY,
                        Count = m.Count,
                        Rezen = m.Rezen,
                        Condition = m.Condition,
                    }).ToList()
                    : new List<MobSpawn>(),
            };

            StatusText = $"복사: {string.Join(", ", options.Where(o => kinds.Contains(o.Kind)).Select(o => o.Label))}. 붙일 칸을 선택하고 Ctrl+V";
        }

        /// <summary>
        /// Ctrl+V: pastes at the top-left of the selection, else at the cell under the mouse, else in the middle of
        /// the view.
        /// </summary>
        public void Paste()
        {
            if (Document == null)
                return;

            if (Clipboard == null)
            {
                StatusText = "붙여넣을 것이 없습니다. 먼저 Ctrl+C로 복사하세요.";
                return;
            }

            var anchors = Selection.ToList();
            foreach (var entity in SelectedEntities)
            {
                if (entity is NpcSpawn npc)
                    anchors.Add((npc.X, npc.Y));
                else if (entity is WarpEntry warp)
                    anchors.Add((warp.X, warp.Y));
                else if (entity is MobSpawn mob)
                    anchors.Add((mob.Left, mob.Top));
            }

            if (anchors.Count > 0)
                PasteAt(anchors.Min(a => a.X), anchors.Min(a => a.Y));
            else if (_hoverCell is (int hx, int hy))
                PasteAt(hx, hy);
            else
                PasteAt(Math.Max(0, (int)ViewCenter.X - Clipboard.Width / 2), Math.Max(0, (int)ViewCenter.Y - Clipboard.Height / 2));
        }

        /// <summary>
        /// Pastes the clipboard with its top-left at (x, y) as one undo step: only the copied cell layers are
        /// written, and copied spawns are added as new ones. Parts outside the map are skipped. The pasted cells and
        /// spawns become the selection so they can be dragged into place.
        /// </summary>
        public void PasteAt(int x, int y)
        {
            if (Document == null || Clipboard == null)
                return;

            var stamp = Clipboard;
            var pasted = new List<Entity>();
            var pastedCells = new List<(int X, int Y)>();
            var before = Document.LastStep;
            using (Document.Group())
            {
                var cells = new List<(int, int, CellValue)>();
                foreach (var (dx, dy, value) in stamp.Cells)
                {
                    var (tx, ty) = (x + dx, y + dy);
                    if (Document.Map.Contains(tx, ty) == false)
                        continue;

                    var cell = Document.Get(tx, ty);
                    if (stamp.Kinds.Contains(CopyKind.Tile))
                        cell.Tile = value.Tile;
                    if (stamp.Kinds.Contains(CopyKind.Object))
                        cell.Object = value.Object;
                    if (stamp.Kinds.Contains(CopyKind.Block))
                        cell.Block = value.Block;
                    cells.Add((tx, ty, cell));
                    pastedCells.Add((tx, ty));
                }
                if (cells.Count > 0)
                    Document.Apply(cells);

                foreach (var source in stamp.Npcs)
                {
                    if (Document.Map.Contains(x + source.X, y + source.Y) == false)
                        continue;

                    var npc = new NpcSpawn { Npc = source.Npc, Info = source.Info, X = x + source.X, Y = y + source.Y, Direction = source.Direction };
                    Document.Npcs.Add(npc);
                    pasted.Add(npc);
                }
                foreach (var source in stamp.Warps)
                {
                    if (Document.Map.Contains(x + source.X, y + source.Y) == false)
                        continue;

                    var warp = new WarpEntry { X = x + source.X, Y = y + source.Y, Dest = source.Dest, Condition = source.Condition, DestName = source.DestName };
                    Document.Warps.Add(warp);
                    pasted.Add(warp);
                }
                foreach (var source in stamp.Mobs)
                {
                    if (Document.Map.Contains(x + source.BeginX, y + source.BeginY) == false)
                        continue;

                    var mob = new MobSpawn
                    {
                        Mob = source.Mob,
                        Info = source.Info,
                        BeginX = x + source.BeginX,
                        BeginY = y + source.BeginY,
                        EndX = Math.Min(Document.Width - 1, x + source.EndX),
                        EndY = Math.Min(Document.Height - 1, y + source.EndY),
                        Count = source.Count,
                        Rezen = source.Rezen,
                        Condition = source.Condition,
                    };
                    Document.Mobs.Add(mob);
                    pasted.Add(mob);
                }
            }

            SelectedEntities.Clear();
            foreach (var entity in pasted)
                SelectedEntities.Add(entity);
            SelectedEntity = pasted.Count == 1 ? pasted[0] : null;
            ChangeSelection(pastedCells, SelectMode.Replace);
            _floatingPaste = (x, y, Document.LastStep, stamp, Document.LastStep != before);
            StatusText = $"붙여넣기: ({x}, {y})에 {pastedCells.Count}칸, 스폰 {pasted.Count}개. 바로 끌면 복사한 층이 모두 함께 이동";
        }

        public void CopyText(string text)
        {
            System.Windows.Clipboard.SetText(text);
            StatusText = $"복사: {text}";
        }

        public void PlaceDoor(int x, int y)
        {
            if (Document == null || SelectedDoorModel == null)
                return;

            var cells = new List<(int, int, CellValue)>();
            for (int i = 0; i < SelectedDoorModel.Pairs.Count; i++)
            {
                var pair = DoorTable.FindPair(SelectedDoorModel.Pairs[i]);
                if (pair == null || Document.Map.Contains(x + i, y) == false)
                    continue;

                var cell = Document.Get(x + i, y);
                cell.Object = (ushort)(PlaceDoorOpened ? pair.Open : pair.Close);
                cells.Add((x + i, y, cell));
            }
            Document.Apply(cells);
        }

        /// <summary>
        /// Same as the server's door::toggle: swap every cell between the open and the close object.
        /// </summary>
        public void ToggleDoor(MapDoor door)
        {
            if (Document == null || door == null)
                return;

            var cells = new List<(int, int, CellValue)>();
            for (int i = 0; i < door.Width; i++)
            {
                var pair = DoorTable.FindPair(door.Model.Pairs[i]);
                var cell = Document.Get(door.X + i, door.Y);
                cell.Object = (ushort)(door.Opened ? pair.Close : pair.Open);
                cells.Add((door.X + i, door.Y, cell));
            }
            Document.Apply(cells);
        }

        public void DeleteDoor(MapDoor door)
        {
            if (Document == null || door == null)
                return;

            EditCells(Rect(door.X, door.Y, door.X + door.Width - 1, door.Y), cell =>
            {
                cell.Object = 0;
                return cell;
            });
        }

        /// <summary>
        /// Cells the door editor works on: a one-row run of selected cells, widened to the whole doors it touches.
        /// Null when the selection is not a single horizontal run (the server only finds doors along rows).
        /// </summary>
        public (int X, int Y, int Width)? DoorEditRange()
        {
            if (Document == null || Selection.Count == 0)
                return null;

            var y = Selection.First().Y;
            var xs = Selection.Select(c => c.X).OrderBy(x => x).ToList();
            if (Selection.Any(c => c.Y != y) || xs[^1] - xs[0] + 1 != xs.Count)
                return null;

            var left = xs[0];
            var right = xs[^1];
            foreach (var door in Document.Doors.Where(d => d.Y == y && d.X <= right && d.X + d.Width - 1 >= left))
            {
                left = Math.Min(left, door.X);
                right = Math.Max(right, door.X + door.Width - 1);
            }
            return (left, y, right - left + 1);
        }

        public void EditDoor()
        {
            if (DoorEditRange() is (int x, int y, int width))
                DoorEditRequested?.Invoke(x, y, width);
            else
                StatusText = "문 편집: 한 행에서 가로로 이어진 칸(오브젝트)을 선택하세요.";
        }

        /// <summary>
        /// Objects that door.xlsx already pairs with obj, most used first: the open objects when obj is a close
        /// object (currentIsClosed), otherwise the close objects.
        /// </summary>
        public List<int> DoorPartners(int obj, bool currentIsClosed)
        {
            if (obj == 0)
                return new List<int>();

            return DoorTable.Pairs.Where(p => (currentIsClosed ? p.Close : p.Open) == obj)
                            .Select(p => currentIsClosed ? p.Open : p.Close)
                            .Where(o => o != obj)
                            .GroupBy(o => o)
                            .OrderByDescending(g => g.Count())
                            .Select(g => g.Key)
                            .Take(8)
                            .ToList();
        }

        /// <summary>
        /// Saves the door made of cells starting at (x, y). door_pair rows with the same open/close objects and a
        /// door model with the same pairs are reused, so doors on other maps keep their definitions; missing ones
        /// are added. With modify, that model's pairs are replaced instead (every map using it changes).
        /// When the server rule would not find the model at (x, y), every change is rolled back (Saved = false) and
        /// Found is the door that wins there instead, or null.
        /// </summary>
        public (DoorModel Model, MapDoor Found, bool Saved) ApplyDoorEdit(int x, int y, IReadOnlyList<(int Open, int Close)> cells, DoorModel modify)
        {
            var dirty = DoorTableDirty;
            var selected = SelectedDoorModel;
            var addedPairs = new List<DoorPair>();
            var ids = new List<int>();
            foreach (var (open, close) in cells)
            {
                var pair = DoorTable.Pairs.FirstOrDefault(p => p.Open == open && p.Close == close);
                if (pair == null)
                {
                    pair = new DoorPair { Id = DoorTable.Pairs.Count == 0 ? 0 : DoorTable.Pairs.Max(p => p.Id) + 1, Open = open, Close = close };
                    pair.PropertyChanged += (s, e) => DoorDefinitionsChanged();
                    DoorTable.Pairs.Add(pair);
                    addedPairs.Add(pair);
                }
                ids.Add(pair.Id);
            }

            DoorModel model;
            DoorModel addedModel = null;
            var oldPairs = modify?.Pairs;
            if (modify != null)
            {
                modify.Pairs = ids;
                model = modify;
            }
            else
            {
                model = DoorTable.Doors.FirstOrDefault(d => d.Pairs.SequenceEqual(ids));
                if (model == null)
                {
                    model = new DoorModel { Id = DoorTable.Doors.Count == 0 ? 0 : DoorTable.Doors.Max(d => d.Id) + 1, Pairs = ids };
                    model.PropertyChanged += (s, e) => DoorDefinitionsChanged();
                    DoorTable.Doors.Add(model);
                    addedModel = model;
                }
            }
            DoorDefinitionsChanged();

            var found = Document?.Doors.FirstOrDefault(d => d.Y == y && d.X == x);
            if (found?.Model == model)
            {
                SelectedDoorModel = model;
                return (model, found, true);
            }
            else
            {
                if (modify != null)
                    modify.Pairs = oldPairs;
                if (addedModel != null)
                    DoorTable.Doors.Remove(addedModel);
                foreach (var pair in addedPairs)
                    DoorTable.Pairs.Remove(pair);
                DoorDefinitionsChanged();
                DoorTableDirty = dirty;
                SelectedDoorModel = selected;
                return (model, found, false);
            }
        }

        private void AddDoorPair()
        {
            var pair = new DoorPair { Id = DoorTable.Pairs.Count == 0 ? 0 : DoorTable.Pairs.Max(p => p.Id) + 1 };
            pair.PropertyChanged += (s, e) => DoorDefinitionsChanged();
            DoorTable.Pairs.Add(pair);
            SelectedDoorPair = pair;
            DoorDefinitionsChanged();
        }

        private void AddDoorModel()
        {
            var model = new DoorModel { Id = DoorTable.Doors.Count == 0 ? 0 : DoorTable.Doors.Max(d => d.Id) + 1 };
            model.PropertyChanged += (s, e) => DoorDefinitionsChanged();
            DoorTable.Doors.Add(model);
            SelectedDoorModel = model;
            DoorDefinitionsChanged();
        }

        private void DoorDefinitionsChanged()
        {
            DoorTableDirty = true;
            DoorRevision++;
            if (Document != null)
            {
                Document.Doors = DoorTable.Find(Document.Width, Document.Height, Document.Map.Objects);
                RefreshDoorModels();
                OnPropertyChanged(nameof(Document));
            }
            OverlayInvalidated?.Invoke();
        }

        private void SaveDoorTable()
        {
            try
            {
                DoorTable.Save();
                DoorTableDirty = false;
                StatusText = "door.xlsx 저장";
            }
            catch (Exception e)
            {
                StatusText = $"door.xlsx 저장 실패: {e.Message}";
            }
        }

        public NpcSpawn AddNpc(int x, int y)
        {
            if (Document == null)
                return null;

            var entry = PickerWindow.Show("NPC 선택", NpcChoices, Assets);
            if (entry == null)
                return null;

            var npc = new NpcSpawn { Npc = entry.Id, Info = entry, X = x, Y = y, Direction = "BOTTOM" };
            Document.Npcs.Add(npc);
            SelectEntity(npc, add: false);
            return npc;
        }

        public MobSpawn AddMob(int x0, int y0, int x1, int y1)
        {
            if (Document == null)
                return null;

            var entry = PickerWindow.Show("몹 선택", MobChoices, Assets);
            if (entry == null)
                return null;

            var mob = new MobSpawn
            {
                Mob = entry.Id,
                Info = entry,
                BeginX = Math.Clamp(Math.Min(x0, x1), 0, Document.Width - 1),
                BeginY = Math.Clamp(Math.Min(y0, y1), 0, Document.Height - 1),
                EndX = Math.Clamp(Math.Max(x0, x1), 0, Document.Width - 1),
                EndY = Math.Clamp(Math.Max(y0, y1), 0, Document.Height - 1),
            };
            Document.Mobs.Add(mob);
            SelectEntity(mob, add: false);
            return mob;
        }

        public WarpEntry AddWarp(int x, int y)
        {
            if (Document == null)
                return null;

            var entry = PickerWindow.Show("목적지 맵 선택", MapChoices);
            if (entry == null)
                return null;

            var warp = new WarpEntry { X = x, Y = y, Dest = $"map({entry.Id}, 0, 0)" };
            Document.Warps.Add(warp);
            SelectEntity(warp, add: false);
            StatusText = "워프 추가: 속성 패널에서 목적지 좌표를 입력하세요.";
            return warp;
        }

        private void AddSpawn(string kind)
        {
            if (Document == null)
                return;

            var x = Document.Width / 2;
            var y = Document.Height / 2;
            Entity added;
            if (kind == "npc")
                added = AddNpc(x, y);
            else if (kind == "mob")
                added = AddMob(0, 0, Document.Width - 1, Document.Height - 1);
            else
                added = AddWarp(x, y);
            FocusEntity(added);
        }

        private void RemoveSpawn(string kind)
        {
            if (kind == "npc")
                RemoveEntity(SelectedNpc);
            else if (kind == "mob")
                RemoveEntity(SelectedMob);
            else
                RemoveEntity(SelectedWarp);
        }

        public void RemoveEntity(Entity entity)
        {
            if (Document == null || entity == null)
                return;

            if (entity is NpcSpawn npc)
                Document.Npcs.Remove(npc);
            else if (entity is MobSpawn mob)
                Document.Mobs.Remove(mob);
            else if (entity is WarpEntry warp)
                Document.Warps.Remove(warp);
        }

        public void FocusEntity(Entity entity)
        {
            if (entity is NpcSpawn npc)
                Jump(npc.X, npc.Y);
            else if (entity is MobSpawn mob)
                Jump((mob.Left + mob.Right) / 2, (mob.Top + mob.Bottom) / 2);
            else if (entity is WarpEntry warp)
                Jump(warp.X, warp.Y);
        }

        /// <summary>
        /// Opens the map of a "map(id, x, y)" destination and centers the destination cell.
        /// </summary>
        public async Task OpenWarpDestination(WarpEntry warp)
        {
            if (warp?.DestMap is not int id)
            {
                StatusText = "map(id, x, y) 형식의 목적지만 열 수 있습니다.";
                return;
            }

            var entry = Maps.FirstOrDefault(m => m.Id == id);
            if (entry == null)
            {
                StatusText = $"맵 {id} 파일이 없습니다.";
                return;
            }

            var x = warp.DestX ?? 0;
            var y = warp.DestY ?? 0;
            await OpenMap(entry);
            if (Document?.Id == id)
            {
                SelectedMap = entry;
                FocusRequested?.Invoke(x, y);
            }
        }

        /// <summary>
        /// Warps that cannot be used: blocked or occupied warp cells, no enterable neighbor, unreachable from the
        /// map's arrival points, broken destinations. all: every map with warps; otherwise the open map. The open map
        /// is checked with its unsaved edits.
        /// </summary>
        public async Task CheckWarps(bool all)
        {
            if (CheckingWarps || Assets == null || _spawns == null || (all == false && Document == null))
                return;

            CheckingWarps = true;
            WarpIssues.Clear();
            StatusText = "워프 검사: 시트 읽는 중...";
            try
            {
                // The open map is checked with its unsaved edits, so copy it here on the UI thread.
                WarpCheckMap current = null;
                if (Document != null)
                {
                    var copy = new ServerMap(Document.Width, Document.Height);
                    Array.Copy(Document.Map.Tiles, copy.Tiles, copy.Tiles.Length);
                    Array.Copy(Document.Map.Objects, copy.Objects, copy.Objects.Length);
                    current = new WarpCheckMap
                    {
                        Id = Document.Id,
                        Name = Document.Name,
                        Map = copy,
                        Blocks = Document.Blocks.Cells.ToHashSet(),
                        Objects = Assets.Objects,
                        DoorCells = Document.Doors.SelectMany(d => Enumerable.Range(d.X, d.Width).Select(x => (x, d.Y))).ToHashSet(),
                        NpcCells = Document.Npcs.Select(n => (n.X, n.Y)).ToHashSet(),
                        Warps = Document.Warps.ToList(),
                    };
                }

                var openNpcs = Document?.Npcs.ToList();
                var directory = Settings.MapDirectory;
                var objects = Assets.Objects;
                var doors = DoorTable;
                var names = _spawns.MapNames;
                var spawns = _spawns;
                var progress = new Progress<string>(text => StatusText = text);
                var (issues, checkedCount) = await Task.Run(() =>
                {
                    var warps = spawns.ReadAllWarps();
                    var npcs = spawns.ReadAllNpcs();
                    if (current != null)
                    {
                        warps[current.Id] = current.Warps;
                        npcs[current.Id] = openNpcs;
                    }

                    var arrivals = new Dictionary<int, List<(int X, int Y)>>();
                    foreach (var warp in warps.Values.SelectMany(w => w))
                    {
                        if (warp.DestMap is int id && warp.DestX is int x && warp.DestY is int y)
                        {
                            if (arrivals.TryGetValue(id, out var list) == false)
                                arrivals[id] = list = new List<(int X, int Y)>();
                            list.Add((x, y));
                        }
                    }

                    var ids = all ? warps.Where(w => w.Value.Count > 0).Select(w => w.Key).OrderBy(id => id).ToList() : new List<int> { current.Id };
                    var loaded = new Dictionary<int, WarpCheckMap>();
                    WarpCheckMap Load(int id)
                    {
                        if (current != null && id == current.Id)
                            return current;
                        if (loaded.TryGetValue(id, out var cached))
                            return cached;

                        var path = Path.Combine(directory, $"{id:000000}.map");
                        WarpCheckMap map = null;
                        if (File.Exists(path))
                        {
                            var serverMap = ServerMap.Read(path);
                            map = new WarpCheckMap
                            {
                                Id = id,
                                Name = names.Find(id),
                                Map = serverMap,
                                Blocks = BlockFile.Read(Path.Combine(directory, $"{id:000000}.block")).Cells.ToHashSet(),
                                Objects = objects,
                                DoorCells = doors.Find(serverMap.Width, serverMap.Height, serverMap.Objects)
                                                 .SelectMany(d => Enumerable.Range(d.X, d.Width).Select(x => (x, d.Y))).ToHashSet(),
                                NpcCells = (npcs.TryGetValue(id, out var list) ? list : new List<NpcSpawn>()).Select(n => (n.X, n.Y)).ToHashSet(),
                                Warps = warps.TryGetValue(id, out var mapWarps) ? mapWarps : new List<WarpEntry>(),
                            };
                        }
                        loaded[id] = map;
                        return map;
                    }

                    var result = new List<WarpIssue>();
                    for (int i = 0; i < ids.Count; i++)
                    {
                        if (i % 100 == 0)
                            ((IProgress<string>)progress).Report($"워프 검사: {i}/{ids.Count} 맵");

                        var map = Load(ids[i]);
                        if (map == null)
                            continue;

                        var mapArrivals = arrivals.TryGetValue(map.Id, out var list) ? list : new List<(int X, int Y)>();
                        result.AddRange(map.Check(mapArrivals, Load, names.Find));
                    }
                    var sorted = result.OrderBy(i => i.Certain ? 0 : 1).ThenBy(i => i.MapId).ThenBy(i => i.Y).ThenBy(i => i.X).ToList();
                    return (sorted, ids.Count);
                });

                foreach (var issue in issues.Take(5000))
                {
                    issue.InOpenMap = issue.MapId == Document?.Id;
                    WarpIssues.Add(issue);
                }
                StatusText = $"워프 검사 ({(all ? $"맵 {checkedCount}개" : current?.Name)}): 확실 {issues.Count(i => i.Certain)}건, 의심 {issues.Count(i => i.Certain == false)}건";
            }
            catch (Exception e)
            {
                StatusText = $"워프 검사 실패: {e.Message}";
            }
            finally
            {
                CheckingWarps = false;
            }
        }

        /// <summary>
        /// Opens the issue's map when needed, centers the warp and selects it.
        /// </summary>
        public async Task FocusWarpIssue(WarpIssue issue)
        {
            if (issue == null)
                return;

            if (Document?.Id == issue.MapId)
            {
                Jump(issue.X, issue.Y);
            }
            else
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

            var warp = WarpAt(issue.X, issue.Y);
            if (warp != null)
                SelectEntity(warp, add: false);
        }

        /// <summary>
        /// Called by PropertyChanged.Fody: only issues of the open map can be checked for deletion.
        /// </summary>
        private void OnDocumentChanged()
        {
            foreach (var issue in WarpIssues)
            {
                issue.InOpenMap = issue.MapId == Document?.Id;
                if (issue.InOpenMap == false)
                    issue.Checked = false;
            }
        }

        /// <summary>
        /// Removes the warps of the checked issues from the open map as one undo step and drops those issues.
        /// </summary>
        public void DeleteCheckedWarps()
        {
            if (Document == null)
                return;

            var issues = WarpIssues.Where(i => i.Checked && i.MapId == Document.Id).ToList();
            if (issues.Count == 0)
            {
                StatusText = "삭제할 워프를 체크하세요. 현재 열린 맵의 항목만 체크할 수 있습니다.";
                return;
            }

            var warps = issues.SelectMany(i => Document.Warps.Where(w => w.X == i.X && w.Y == i.Y && w.Dest == i.Dest)).Distinct().ToList();
            using (Document.Group())
            {
                foreach (var warp in warps)
                    Document.Warps.Remove(warp);
            }
            foreach (var warp in warps)
                SelectedEntities.Remove(warp);
            if (SelectedEntity is WarpEntry selected && warps.Contains(selected))
                SelectedEntity = null;
            foreach (var issue in WarpIssues.Where(i => i.MapId == Document.Id && warps.Any(w => w.X == i.X && w.Y == i.Y && w.Dest == i.Dest)).ToList())
                WarpIssues.Remove(issue);
            UpdateSelectionText();
            OverlayInvalidated?.Invoke();
            StatusText = $"워프 {warps.Count}개 삭제 (실행 취소 가능)";
        }

        public List<ValidationItem> Validate()
        {
            Validation.Clear();
            if (Document == null)
                return new List<ValidationItem>();

            var doc = Document;
            var items = new List<ValidationItem>();
            foreach (var npc in doc.Npcs)
            {
                if (doc.Map.Contains(npc.X, npc.Y) == false)
                    items.Add(new ValidationItem { X = npc.X, Y = npc.Y, Message = $"NPC {npc.Npc} {npc.Name}: 맵 밖" });
                else if (IsBlocked(npc.X, npc.Y))
                    items.Add(new ValidationItem { X = npc.X, Y = npc.Y, Message = $"NPC {npc.Npc} {npc.Name}: 막힌 칸" });
            }
            foreach (var group in doc.Npcs.GroupBy(n => (n.X, n.Y)).Where(g => g.Count() > 1))
                items.Add(new ValidationItem { X = group.Key.X, Y = group.Key.Y, Message = $"NPC {group.Count()}개가 같은 칸" });

            foreach (var mob in doc.Mobs)
            {
                if (doc.Map.Contains(mob.BeginX, mob.BeginY) == false || doc.Map.Contains(mob.EndX, mob.EndY) == false)
                {
                    items.Add(new ValidationItem { X = mob.BeginX, Y = mob.BeginY, Message = $"몹 {mob.Mob} {mob.Name}: 스폰 영역이 맵 밖" });
                }
                else
                {
                    var open = Rect(mob.BeginX, mob.BeginY, mob.EndX, mob.EndY).Any(c => IsBlocked(c.X, c.Y) == false);
                    if (open == false)
                        items.Add(new ValidationItem { X = mob.BeginX, Y = mob.BeginY, Message = $"몹 {mob.Mob} {mob.Name}: 스폰 영역이 전부 막힘" });
                }
            }
            foreach (var warp in doc.Warps)
            {
                if (doc.Map.Contains(warp.X, warp.Y) == false)
                    items.Add(new ValidationItem { X = warp.X, Y = warp.Y, Message = $"워프 {warp.Dest} {warp.DestName}: 출발 칸이 맵 밖" });
                if (warp.DestMap is int destMap && Maps.Any(m => m.Id == destMap) == false)
                    items.Add(new ValidationItem { X = warp.X, Y = warp.Y, Message = $"워프 {warp.Dest} {warp.DestName}: 목적지 맵 파일 없음" });
            }
            foreach (var (x, y) in doc.Blocks.Cells)
            {
                if (doc.Map.Contains(x, y) == false)
                    items.Add(new ValidationItem { X = x, Y = y, Message = "블록 좌표가 맵 밖" });
            }
            if (Assets != null)
            {
                for (int y = 0; y < doc.Height; y++)
                {
                    for (int x = 0; x < doc.Width; x++)
                    {
                        var cell = doc.Get(x, y);
                        if (cell.Tile >= Assets.TileCount)
                            items.Add(new ValidationItem { X = x, Y = y, Message = $"타일 {cell.Tile}이 {Assets.Version} 리소스 범위 밖" });
                        if (cell.Object > Assets.Objects.Count)
                            items.Add(new ValidationItem { X = x, Y = y, Message = $"오브젝트 {cell.Object}가 {Assets.Version} 리소스 범위 밖" });
                    }
                }
            }
            foreach (var model in DoorTable.Doors)
            {
                foreach (var pairId in model.Pairs)
                {
                    var pair = DoorTable.FindPair(pairId);
                    if (pair == null)
                        items.Add(new ValidationItem { Message = $"문 {model.Id}: door_pair {pairId} 없음" });
                    else if (pair.Open == pair.Close)
                        items.Add(new ValidationItem { Message = $"door_pair {pair.Id}: 열림과 닫힘 오브젝트가 같음" });
                }
            }

            foreach (var item in items.Take(2000))
                Validation.Add(item);
            StatusText = $"검증: {items.Count}건";
            return items;
        }
    }
}
