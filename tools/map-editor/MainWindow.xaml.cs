using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using MapEditor.Command;
using MapEditor.Edit;
using MapEditor.Table;
using MapEditor.ViewModel;

namespace MapEditor
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            DataContextChanged += (s, e) =>
            {
                if (e.OldValue is MainWindowViewModel old)
                {
                    old.MapSearchRequested -= FocusMapSearch;
                    old.ShortcutEditorRequested -= EditShortcuts;
                    old.PropertyChanged -= OnEditorPropertyChanged;
                }
                if (e.NewValue is MainWindowViewModel editor)
                {
                    editor.MapSearchRequested += FocusMapSearch;
                    editor.ShortcutEditorRequested += EditShortcuts;
                    editor.PropertyChanged += OnEditorPropertyChanged;
                }
            };
        }

        private void OnEditorPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(MainWindowViewModel.CheckingWarps) && Editor.CheckingWarps)
                WarpCheckTab.IsSelected = true;
        }

        private MainWindowViewModel Editor => (MainWindowViewModel)DataContext;

        /// <summary>
        /// Keys no control handled go to the user's shortcuts. Gestures without Ctrl/Alt are left to a focused text box.
        /// </summary>
        protected override void OnKeyDown(KeyEventArgs e)
        {
            base.OnKeyDown(e);
            if (e.Handled || DataContext is not MainWindowViewModel editor)
                return;

            var key = e.Key == Key.System ? e.SystemKey : e.Key == Key.ImeProcessed ? e.ImeProcessedKey : e.Key;
            if (Gesture.IsModifierKey(key))
                return;

            // A focused grid cell starts editing on typed text, so it counts as typing too.
            var typing = Keyboard.FocusedElement is TextBoxBase or PasswordBox or ComboBox { IsEditable: true } or DataGridCell;
            e.Handled = editor.RunShortcut(new Gesture(Keyboard.Modifiers, key, null), typing);
        }

        /// <summary>
        /// Mouse side buttons (back/forward by default) work anywhere in the window.
        /// </summary>
        protected override void OnPreviewMouseDown(MouseButtonEventArgs e)
        {
            base.OnPreviewMouseDown(e);
            if (DataContext is MainWindowViewModel editor && Gesture.IsGestureButton(e.ChangedButton))
                e.Handled = editor.RunShortcut(new Gesture(Keyboard.Modifiers, Key.None, e.ChangedButton), typing: false);
        }

        private void FocusMapSearch()
        {
            MapQueryBox.Focus();
            MapQueryBox.SelectAll();
        }

        private void OnFindMap(object sender, RoutedEventArgs e)
        {
            FocusMapSearch();
        }

        private void OnMapQueryKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && MapList.Items.Count > 0)
            {
                // The query binding is delayed; filter now so Enter opens what is typed.
                MapQueryBox.GetBindingExpression(TextBox.TextProperty)?.UpdateSource();
                if (MapList.Items.Count > 0 && (MapList.SelectedItem ?? MapList.Items[0]) is MapEntry entry)
                    _ = Editor.OpenMap(entry);
                e.Handled = true;
            }
            else if (e.Key == Key.Down && MapList.Items.Count > 0)
            {
                MapList.SelectedIndex = Math.Max(0, MapList.SelectedIndex);
                (MapList.ItemContainerGenerator.ContainerFromIndex(MapList.SelectedIndex) as ListBoxItem)?.Focus();
                e.Handled = true;
            }
        }

        private void EditShortcuts()
        {
            if (ShortcutWindow.Edit(this, Editor.Shortcuts))
                Editor.SaveShortcuts();
        }

        private void OnMapDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (MapList.SelectedItem is MapEntry entry)
                _ = Editor.OpenMap(entry);
        }

        private void OnDoorDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (Editor.SelectedMapDoor != null)
                Editor.Jump(Editor.SelectedMapDoor.X, Editor.SelectedMapDoor.Y);
        }

        private void OnDoorUsageClick(object sender, MouseButtonEventArgs e)
        {
            if (sender is ListBox list && list.SelectedItem is MapDoor door)
                Editor.Jump(door.X, door.Y);
        }

        private void OnWarpIssueClick(object sender, MouseButtonEventArgs e)
        {
            if (sender is DataGrid grid && grid.SelectedItem is WarpIssue issue)
                _ = Editor.FocusWarpIssue(issue);
        }

        private void OnNpcDoubleClick(object sender, MouseButtonEventArgs e)
        {
            Editor.FocusEntity(Editor.SelectedNpc);
        }

        private void OnMobDoubleClick(object sender, MouseButtonEventArgs e)
        {
            Editor.FocusEntity(Editor.SelectedMob);
        }

        private void OnWarpDoubleClick(object sender, MouseButtonEventArgs e)
        {
            Editor.FocusEntity(Editor.SelectedWarp);
        }

        private void OnValidationDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (sender is ListBox list && list.SelectedItem is ValidationItem item)
                Editor.Jump(item.X, item.Y);
        }

        private void OnHelp(object sender, RoutedEventArgs e)
        {
            var shortcuts = string.Join("\n", Editor.Shortcuts.Where(s => s.Text != "").Select(s => $"  {s.Label}: {s.Text}"));
            MessageBox.Show(
                "선택 / 이동 도구\n" +
                "  좌클릭 끌기: NPC, 워프, 오브젝트, 몹 영역 테두리, 선택 영역을 이동\n" +
                "  선택한 몹 영역의 꼭짓점 끌기: 영역 크기 조절\n" +
                "  빈 곳 끌기: 영역 선택 (안의 오브젝트, NPC, 워프, 몹 영역 포함)\n" +
                "  Ctrl + 클릭: 그 항목을 선택에 추가 / 빼기\n" +
                "  Shift 또는 Ctrl + 끌기: 영역 추가, Ctrl + Shift + 끌기: 영역 제외\n" +
                "  더블클릭: 현재 레이어에서 같은 값 전체 선택\n" +
                "  클릭은 항상 그 칸을 선택. Alt + 클릭: 위로 뻗은 키 큰 오브젝트를 그림 위치로 잡기 (밑동 칸 선택)\n" +
                "  방향키: 선택을 한 칸 이동\n\n" +
                "우클릭: 그 칸의 작업 메뉴 (NPC/워프/몹 추가, 블록, 스포이트, 문 등)\n" +
                "우클릭 끌기: 영역 작업 메뉴 (몹 스폰 추가, 블록 설정/해제, 채우기 등)\n" +
                "가운데 버튼 끌기 또는 Space + 끌기: 화면 이동\n" +
                "Ctrl + 휠: 확대 / 축소, Shift + 휠: 가로 스크롤\n" +
                "미니맵 클릭 / 끌기: 그 위치로 화면 이동\n" +
                "뒤로 / 앞으로: 맵 열기, 워프 목적지 열기, 목록에서 좌표로 이동한 기록을 오감\n\n" +
                "단축키 (설정 → 단축키에서 변경)\n" + shortcuts,
                "조작 안내");
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);
            if (DataContext is not MainWindowViewModel editor)
                return;

            var dirty = new List<string>();
            if (editor.Document != null && editor.Document.Dirty)
                dirty.Add(editor.Document.Title);
            if (editor.DoorTableDirty)
                dirty.Add("door.xlsx");
            if (dirty.Count == 0)
                return;

            var answer = MessageBox.Show($"저장하지 않은 변경 사항이 있습니다.\n{string.Join("\n", dirty)}\n\n저장하지 않고 닫을까요?",
                                         "맵 에디터", MessageBoxButton.YesNo, MessageBoxImage.Warning);
            if (answer != MessageBoxResult.Yes)
                e.Cancel = true;
        }
    }
}
