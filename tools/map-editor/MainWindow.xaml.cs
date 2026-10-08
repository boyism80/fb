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
                    old.IssuesRequested -= ShowIssues;
                    old.DoorEditRequested -= ShowDoorEdit;
                    old.NewMapRequested -= ShowNewMap;
                    old.SaveAsRequested -= ShowSaveAs;
                    old.TemplateScanRequested -= ShowTemplateScan;
                    old.TemplateWarpCheckRequested -= ShowTemplateWarpCheck;
                    old.GoToRequested -= ShowGoTo;
                }
                if (e.NewValue is MainWindowViewModel editor)
                {
                    editor.MapSearchRequested += FocusMapSearch;
                    editor.ShortcutEditorRequested += EditShortcuts;
                    editor.IssuesRequested += ShowIssues;
                    editor.DoorEditRequested += ShowDoorEdit;
                    editor.NewMapRequested += ShowNewMap;
                    editor.SaveAsRequested += ShowSaveAs;
                    editor.TemplateScanRequested += ShowTemplateScan;
                    editor.TemplateWarpCheckRequested += ShowTemplateWarpCheck;
                    editor.GoToRequested += ShowGoTo;
                    MinimapImage.Side = Math.Clamp(editor.User.MinimapSide, 100, 800);
                    PlaceMinimap();
                }
            };
            MapScroll.SizeChanged += (s, e) => PlaceMinimap();
            Minimap.SizeChanged += (s, e) => PlaceMinimap();
            MapScroll.ScrollChanged += (s, e) =>
            {
                if (e.ViewportWidthChange != 0 || e.ViewportHeightChange != 0)
                    PlaceMinimap();
            };
        }

        private TemplateWarpWindow _templateWarp;
        private Point? _minimapDrag;
        private Point _minimapStart;
        private double _minimapSideStart;

        private void ShowNewMap()
        {
            _ = NewMapWindow.Show(Editor);
        }

        private void ShowGoTo()
        {
            _ = GoToWindow.Show(this, Editor);
        }

        private void OnGoTo(object sender, RoutedEventArgs e)
        {
            ShowGoTo();
        }

        private void ShowTemplateScan()
        {
            new TemplateScanWindow(Editor) { Owner = this }.ShowDialog();
        }

        private void ShowTemplateWarpCheck()
        {
            if (_templateWarp == null)
            {
                _templateWarp = new TemplateWarpWindow(Editor) { Owner = this };
                _templateWarp.Closed += (s, e) => _templateWarp = null;
                _templateWarp.Show();
            }
            _templateWarp.Activate();
        }

        private void OnMapListKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Delete && Keyboard.Modifiers == ModifierKeys.None)
            {
                Editor.DeleteMapsCommand.Execute(MapList.SelectedItems);
                e.Handled = true;
            }
            else if (e.Key == Key.Enter && MapList.SelectedItem is MapEntry entry)
            {
                _ = Editor.OpenMap(entry);
                e.Handled = true;
            }
        }

        private void OnMapTabChanged(object sender, SelectionChangedEventArgs e)
        {
            if (MapTabs.SelectedItem is MapDocument document && document != Editor.Document)
                Editor.Activate(document);
            MapTabs.ScrollIntoView(Editor.Document);
        }

        private void OnMapTabMouseUp(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Middle && (e.OriginalSource as FrameworkElement)?.DataContext is MapDocument document)
            {
                Editor.CloseDocument(document);
                e.Handled = true;
            }
        }

        private void OnCloseMapTab(object sender, RoutedEventArgs e)
        {
            if ((sender as FrameworkElement)?.DataContext is MapDocument document)
                Editor.CloseDocument(document);
        }

        private void OnMapTabMenu(object sender, RoutedEventArgs e)
        {
            if (sender is not MenuItem item || item.DataContext is not MapDocument document)
                return;

            var tabs = Editor.OpenDocuments.ToList();
            var index = tabs.IndexOf(document);
            var path = System.IO.Path.Combine(Editor.Settings.MapDirectory, $"{document.Id:000000}.map");
            switch (item.Tag as string)
            {
                case "save":
                    Editor.Save(document);
                    break;
                case "saveAs":
                    Editor.RequestSaveAs(document);
                    break;
                case "saveAll":
                    Editor.SaveAll();
                    break;
                case "close":
                    Editor.CloseDocument(document);
                    break;
                case "closeOthers":
                    Editor.CloseDocuments(tabs.Where(d => d != document));
                    break;
                case "closeRight":
                    Editor.CloseDocuments(tabs.Skip(index + 1));
                    break;
                case "closeLeft":
                    Editor.CloseDocuments(tabs.Take(index));
                    break;
                case "closeSaved":
                    Editor.CloseDocuments(tabs.Where(d => d.Dirty == false));
                    break;
                case "closeAll":
                    Editor.CloseDocuments(tabs);
                    break;
                case "reveal":
                    var entry = Editor.Maps.FirstOrDefault(m => m.Id == document.Id);
                    if (entry == null)
                        break;
                    if (Editor.MapView.Contains(entry) == false)
                        Editor.MapQuery = "";
                    Editor.SelectedMap = entry;
                    MapList.ScrollIntoView(entry);
                    break;
                case "copyPath":
                    Clipboard.SetText(path);
                    Editor.StatusText = $"복사: {path}";
                    break;
                case "explorer":
                    System.Diagnostics.Process.Start("explorer.exe", System.IO.File.Exists(path) ? $"/select,\"{path}\"" : $"\"{Editor.Settings.MapDirectory}\"");
                    break;
            }
        }

        private void OnSelectOptions(object sender, RoutedEventArgs e)
        {
            if (sender is not FrameworkElement button || (button.Parent as FrameworkElement)?.ContextMenu is not ContextMenu menu)
                return;

            menu.PlacementTarget = button;
            menu.Placement = PlacementMode.Bottom;
            menu.IsOpen = true;
        }

        private void ShowSaveAs(MapDocument document)
        {
            _ = NewMapWindow.ShowSaveAs(Editor, document);
        }

        private void OnTemplateDoubleClick(object sender, MouseButtonEventArgs e)
        {
            Editor.PlaceTemplateCommand.Execute(null);
        }

        private void OnTemplateListKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Delete && Keyboard.Modifiers == ModifierKeys.None)
            {
                Editor.DeleteTemplatesCommand.Execute(TemplateList.SelectedItems);
                e.Handled = true;
            }
        }

        private void OnMinimapDragStart(object sender, MouseButtonEventArgs e)
        {
            _minimapDrag = e.GetPosition(MapScroll);
            _minimapStart = Minimap.TranslatePoint(new Point(0, 0), MapScroll);
            ((UIElement)sender).CaptureMouse();
            e.Handled = true;
        }

        private void OnMinimapDrag(object sender, MouseEventArgs e)
        {
            if (_minimapDrag is not Point start)
                return;

            var delta = e.GetPosition(MapScroll) - start;
            MoveMinimap(_minimapStart.X + delta.X, _minimapStart.Y + delta.Y, snap: true);
        }

        private void OnMinimapResizeStart(object sender, MouseButtonEventArgs e)
        {
            _minimapDrag = e.GetPosition(MapScroll);
            _minimapStart = Minimap.TranslatePoint(new Point(0, 0), MapScroll);
            _minimapSideStart = MinimapImage.Side;
            ((UIElement)sender).CaptureMouse();
            e.Handled = true;
        }

        private void OnMinimapResize(object sender, MouseEventArgs e)
        {
            if (_minimapDrag is not Point start)
                return;

            // The top-left corner stays where it is; the grip follows the mouse along the larger movement.
            var delta = e.GetPosition(MapScroll) - start;
            var grow = Math.Abs(delta.X) > Math.Abs(delta.Y) ? delta.X : delta.Y;
            var (viewWidth, viewHeight) = MapViewport();
            MinimapImage.Side = Math.Clamp(_minimapSideStart + grow, 100, Math.Max(100, Math.Min(800, Math.Min(viewWidth, viewHeight) - 40)));
            Minimap.UpdateLayout();
            MoveMinimap(_minimapStart.X, _minimapStart.Y, snap: false);
        }

        private void OnMinimapDragEnd(object sender, MouseButtonEventArgs e)
        {
            if (_minimapDrag == null)
                return;

            _minimapDrag = null;
            ((UIElement)sender).ReleaseMouseCapture();
            Editor.User.MinimapSide = MinimapImage.Side;
            try
            {
                Editor.User.Save();
            }
            catch (Exception ex)
            {
                Editor.StatusText = $"미니맵 위치 저장 실패: {ex.Message}";
            }
        }

        /// <summary>
        /// Visible part of the map view, without its scroll bars.
        /// </summary>
        private (double Width, double Height) MapViewport()
        {
            return (MapScroll.ViewportWidth > 0 ? MapScroll.ViewportWidth : MapScroll.ActualWidth,
                    MapScroll.ViewportHeight > 0 ? MapScroll.ViewportHeight : MapScroll.ActualHeight);
        }

        /// <summary>
        /// Puts the minimap's top-left corner at (left, top) of the map view, kept inside it. With snap, an edge
        /// closer than MinimapSnap to a view edge docks onto it. The minimap is anchored to the nearest corner so it
        /// stays docked when the view is resized.
        /// </summary>
        private void MoveMinimap(double left, double top, bool snap)
        {
            const double MinimapSnap = 20;
            var (viewWidth, viewHeight) = MapViewport();
            var width = Minimap.ActualWidth;
            var height = Minimap.ActualHeight;
            left = Math.Clamp(left, 0, Math.Max(0, viewWidth - width));
            top = Math.Clamp(top, 0, Math.Max(0, viewHeight - height));
            if (snap)
            {
                if (left < MinimapSnap)
                    left = 0;
                else if (viewWidth - width - left < MinimapSnap)
                    left = Math.Max(0, viewWidth - width);

                if (top < MinimapSnap)
                    top = 0;
                else if (viewHeight - height - top < MinimapSnap)
                    top = Math.Max(0, viewHeight - height);
            }

            var user = Editor.User;
            user.MinimapLeft = left + width / 2 < viewWidth / 2;
            user.MinimapTop = top + height / 2 < viewHeight / 2;
            user.MinimapOffsetX = user.MinimapLeft ? left : viewWidth - width - left;
            user.MinimapOffsetY = user.MinimapTop ? top : viewHeight - height - top;
            PlaceMinimap();
        }

        /// <summary>
        /// Applies the saved corner and offsets, pulled in when the view became too small to hold the minimap there.
        /// </summary>
        private void PlaceMinimap()
        {
            if (DataContext is not MainWindowViewModel editor)
                return;

            var user = editor.User;
            var (viewWidth, viewHeight) = MapViewport();
            var x = Math.Clamp(user.MinimapOffsetX, 0, Math.Max(0, viewWidth - Minimap.ActualWidth));
            var y = Math.Clamp(user.MinimapOffsetY, 0, Math.Max(0, viewHeight - Minimap.ActualHeight));
            var scrollRight = MapScroll.ActualWidth - viewWidth;
            var scrollBottom = MapScroll.ActualHeight - viewHeight;
            Minimap.HorizontalAlignment = user.MinimapLeft ? HorizontalAlignment.Left : HorizontalAlignment.Right;
            Minimap.VerticalAlignment = user.MinimapTop ? VerticalAlignment.Top : VerticalAlignment.Bottom;
            Minimap.Margin = new Thickness(user.MinimapLeft ? x : 0, user.MinimapTop ? y : 0, user.MinimapLeft ? 0 : x + scrollRight, user.MinimapTop ? 0 : y + scrollBottom);
        }

        private MainWindowViewModel Editor => (MainWindowViewModel)DataContext;

        private IssuesWindow _issues;
        private DoorEditorWindow _doorEditor;
        private McpWindow _mcp;
        private DoorEditWindow _doorEdit;

        private void ShowDoorEdit(int x, int y, int width)
        {
            _doorEdit?.Close();
            _doorEdit = new DoorEditWindow(this, Editor, x, y, width);
            _doorEdit.Closed += (s, args) =>
            {
                if (ReferenceEquals(_doorEdit, s))
                    _doorEdit = null;
            };
            _doorEdit.Show();
        }

        private void ShowIssues(IssueTab tab)
        {
            if (_issues == null)
            {
                _issues = new IssuesWindow(this, Editor);
                _issues.Closed += (s, e) => _issues = null;
            }
            _issues.ShowTab(tab);
        }

        private void OnIssues(object sender, RoutedEventArgs e)
        {
            ShowIssues(IssueTab.Warps);
        }

        private void OnDoorEditor(object sender, RoutedEventArgs e)
        {
            if (_doorEditor == null)
            {
                _doorEditor = new DoorEditorWindow(this, Editor);
                _doorEditor.Closed += (s, args) => _doorEditor = null;
                _doorEditor.Show();
            }
            _doorEditor.Activate();
        }

        private void OnMcp(object sender, RoutedEventArgs e)
        {
            if (_mcp == null)
            {
                _mcp = new McpWindow(this, Editor);
                _mcp.Closed += (s, args) => _mcp = null;
                _mcp.Show();
            }
            _mcp.Activate();
        }

        /// <summary>
        /// Enter in a palette id box commits the id so the palette scrolls to it.
        /// </summary>
        private void OnIdBoxKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && sender is TextBox box)
            {
                box.GetBindingExpression(TextBox.TextProperty)?.UpdateSource();
                box.SelectAll();
                e.Handled = true;
            }
        }

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
            if (sender is ListBox list && list.SelectedItem is DoorReference reference)
                _ = Editor.GoToDoorReference(reference);
        }

        private void OnTemplatePlaceClick(object sender, MouseButtonEventArgs e)
        {
            if (sender is ListBox list && list.SelectedItem is TemplatePlace place)
                _ = Editor.GoToTemplatePlace(place);
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

        private void OnHelp(object sender, RoutedEventArgs e)
        {
            var shortcuts = string.Join("\n", Editor.Shortcuts.Where(s => s.Text != "").Select(s => $"  {s.Label}: {s.Text}"));
            MessageBox.Show(
                "선택 / 이동 도구\n" +
                "  클릭: NPC, 워프, 몹 영역 테두리를 선택. 그 밖에는 그 칸을 선택\n" +
                "  선택한 것(칸, NPC, 워프, 몹 영역) 위에서 끌기: 이동\n" +
                "  그 밖의 곳에서 끌기: 영역 선택 (안의 오브젝트, NPC, 워프, 몹 영역 포함). 오브젝트가 있어도 영역 선택\n" +
                "  Alt + 끌기: 선택하지 않은 것도 바로 집어서 이동. 키 큰 오브젝트는 그림 위치로 잡힘 (밑동 칸)\n" +
                "  선택한 몹 영역의 꼭짓점 끌기: 영역 크기 조절\n" +
                "  Ctrl + 클릭: 그 항목을 선택에 추가 / 빼기\n" +
                "  Shift 또는 Ctrl + 끌기: 영역 추가, Ctrl + Shift + 끌기: 영역 제외\n" +
                "  더블클릭: 현재 레이어에서 같은 값 전체 선택\n" +
                "  방향키: 선택을 한 칸 이동\n\n" +
                "우클릭: 그 칸의 작업 메뉴 (NPC/워프/몹 추가, 블록, 스포이트, 문 등)\n" +
                "우클릭 끌기: 영역 작업 메뉴 (몹 스폰 추가, 블록 설정/해제, 채우기 등). 메뉴가 열린 동안 대상 영역 표시\n" +
                "맵 → 문제 검사 창: 워프 검사(현재 맵 항목은 체크해서 삭제)와 맵 검증\n" +
                "맵 → 문 정의 편집기: door / door_pair 표 편집과 door.xlsx 저장\n" +
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

            var dirty = editor.OpenDocuments.Where(d => d.Dirty).Select(d => d.Title).ToList();
            if (editor.DoorTableDirty)
                dirty.Add("door.xlsx");
            if (dirty.Count > 0)
            {
                var answer = MessageBox.Show($"저장하지 않은 변경 사항이 있습니다.\n{string.Join("\n", dirty)}\n\n저장하지 않고 닫을까요?",
                                             "맵 에디터", MessageBoxButton.YesNo, MessageBoxImage.Warning);
                if (answer != MessageBoxResult.Yes)
                {
                    e.Cancel = true;
                    return;
                }
            }

            try
            {
                editor.SaveSession();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"열린 탭과 위치를 저장하지 못했습니다: {ex.Message}", "맵 에디터", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
        }
    }
}
