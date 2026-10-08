using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;
using MapEditor.Asset;
using MapEditor.Table;
using MapEditor.ViewModel;

namespace MapEditor
{
    /// <summary>
    /// One door cell: the object on the map now and the object of the other state.
    /// </summary>
    public class DoorEditCell : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public int X { get; init; }
        public int Current { get; set; }
        public int Partner { get; set; }
        public bool CurrentIsClosed { get; set; } = true;

        public string Header => $"x {X}";
        public int OpenId => CurrentIsClosed ? Partner : Current;
        public int CloseId => CurrentIsClosed ? Current : Partner;

        /// <summary>
        /// Both states show the same object: a frame piece that stays while the rest of the door opens.
        /// </summary>
        public bool Fixed => Partner == Current;

        /// <summary>
        /// The other state has no object, so this cell disappears when the door toggles.
        /// </summary>
        public bool Empty => Partner == 0 && Current != 0;

        public string PartnerLabel => Fixed ? $"#{Partner} 고정" : Partner == 0 ? "#0 (없음)" : $"#{Partner}";
    }

    /// <summary>
    /// Pairs each selected cell's object with the object of the other door state and saves the door definition.
    /// </summary>
    public partial class DoorEditWindow : Window
    {
        private readonly MainWindowViewModel _editor;
        private readonly List<DoorEditCell> _cells;
        private readonly int _x;
        private readonly int _y;
        private readonly DoorModel _existing;
        private MapDoor _applied;
        private List<(int Open, int Close)> _original;

        public DoorEditWindow(Window owner, MainWindowViewModel editor, int x, int y, int width)
        {
            InitializeComponent();
            Owner = owner;
            DataContext = editor;
            _editor = editor;
            _x = x;
            _y = y;

            var doc = editor.Document;
            var doors = doc.Doors.Where(d => d.Y == y && d.X < x + width && d.X + d.Width > x).ToList();
            _cells = Enumerable.Range(x, width).Select(cx => new DoorEditCell { X = cx, Current = doc.Get(cx, y).Object }).ToList();

            // Cells that are already part of a door start with that door's other state.
            var closed = doors.Count == 0 || doors[0].Opened == false;
            foreach (var cell in _cells)
            {
                cell.CurrentIsClosed = closed;
                cell.Partner = cell.Current;
                var door = doors.FirstOrDefault(d => cell.X >= d.X && cell.X < d.X + d.Width);
                var pair = door == null ? null : editor.DoorTable.FindPair(door.Model.Pairs[cell.X - door.X]);
                if (pair != null)
                    cell.Partner = door.Opened ? pair.Close : pair.Open;
            }

            _existing = doors.Count == 1 && doors[0].X == x && doors[0].Width == width ? doors[0].Model : null;
            Heading.Text = $"문 편집 — {doc.Title}  ({x}, {y}) ~ ({x + width - 1}, {y}), {width}칸";
            if (doors.Count == 0)
            {
                CurrentInfo.Text = "지금은 문이 아닙니다. 각 칸의 짝(반대 상태 오브젝트)을 고르고 적용하면 문 정의가 만들어집니다.";
            }
            else
            {
                var same = doors.Select(d => d.Model).Distinct().Select(m => $"문 {m.Id} (이 맵에서 {doc.Doors.Count(d => d.Model == m)}곳)");
                CurrentInfo.Text = $"지금 인식되는 문: {string.Join(", ", same)}, {(closed ? "닫힘" : "열림")} 상태. 짝을 바꾸고 적용하면 같은 구성의 정의를 찾거나 새로 만듭니다.";
            }
            _applied = _existing != null ? doors[0] : null;
            ToggleButton.IsEnabled = _applied != null;
            if (_existing != null)
            {
                ModifyExisting.Content = $"새 정의를 만들지 않고 문 {_existing.Id}의 정의를 직접 수정 (이 정의를 쓰는 다른 맵의 문도 함께 바뀜)";
                ModifyExisting.Visibility = Visibility.Visible;
            }

            Columns.ItemsSource = _cells;
            if (closed)
                CurrentClosed.IsChecked = true;
            else
                CurrentOpened.IsChecked = true;
            _original = doors.Count == 0 ? new List<(int, int)>() : _cells.Select(c => (c.OpenId, c.CloseId)).ToList();
            foreach (var cell in _cells)
                cell.PropertyChanged += (s, e) => Refresh();
            Refresh();
        }

        private bool CurrentIsClosed => CurrentClosed.IsChecked == true;

        private void OnStateChanged(object sender, RoutedEventArgs e)
        {
            if (_cells == null)
                return;

            foreach (var cell in _cells)
                cell.CurrentIsClosed = CurrentIsClosed;
            CurrentRowLabel.Text = CurrentIsClosed ? "현재 (닫힘)" : "현재 (열림)";
            PartnerRowLabel.Text = CurrentIsClosed ? "짝 (열림)" : "짝 (닫힘)";
            Refresh();
        }

        private void OnPreviewChanged(object sender, RoutedEventArgs e)
        {
            if (_cells != null)
                Refresh();
        }

        private void Refresh()
        {
            var opened = PreviewOpened.IsChecked == true;
            var assets = _editor.Assets;
            Preview.Source = assets == null ? null : Thumbnail.Row(assets, _cells.Select(c => opened ? c.OpenId : c.CloseId).ToList());
            PreviewState.Text = opened ? "열린 문" : "닫힌 문";
            PreviewState.Foreground = opened ? Brushes.LimeGreen : Brushes.Gold;

            var problems = new List<string>();
            if (_cells.All(c => c.Fixed))
                problems.Add("모든 칸이 고정이라 열고 닫아도 바뀌는 것이 없습니다. 바뀔 칸의 짝을 고르세요.");
            var empty = _cells.Where(c => c.Empty).Select(c => c.X).ToList();
            if (empty.Count > 0)
                problems.Add($"x {string.Join(", ", empty)}: 반대 상태에서는 오브젝트가 사라집니다 (의도한 것이 아니면 짝을 고르세요).");
            var fixedCount = _cells.Count(c => c.Fixed);
            Message.Foreground = problems.Count > 0 ? Brushes.Orange : (Brush)FindResource("SemiTextBrush");
            Message.Text = problems.Count > 0
                ? string.Join("\n", problems)
                : $"바뀌는 칸 {_cells.Count - fixedCount}, 고정 칸 {fixedCount}. 적용하면 door.xlsx 표가 바뀝니다. 파일 저장은 'door.xlsx 저장'.";
        }

        private void OnSlotClick(object sender, RoutedEventArgs e)
        {
            if (sender is not Button button || button.DataContext is not DoorEditCell cell)
                return;

            var assets = _editor.Assets;
            var menu = new ContextMenu { PlacementTarget = button, Placement = PlacementMode.Bottom };
            var partners = _editor.DoorPartners(cell.Current, CurrentIsClosed);
            menu.Items.Add(new MenuItem { Header = partners.Count > 0 ? "door.xlsx에서 이 오브젝트와 짝인 것" : "door.xlsx에 이 오브젝트의 짝 없음", IsEnabled = false });
            foreach (var id in partners)
                menu.Items.Add(ObjectItem(assets, id, $"#{id}", cell));

            menu.Items.Add(new Separator());
            menu.Items.Add(ObjectItem(assets, _editor.SelectedObject, $"팔레트 선택값 #{_editor.SelectedObject}", cell));

            var near = new MenuItem { Header = "인접 id (문 그림은 보통 붙어 있음)" };
            foreach (var id in new[] { cell.Current - 2, cell.Current - 1, cell.Current + 1, cell.Current + 2 }.Where(id => id > 0 && id <= (assets?.Objects.Count ?? 0)))
                near.Items.Add(ObjectItem(assets, id, $"#{id}", cell));
            menu.Items.Add(near);

            menu.Items.Add(new Separator());
            menu.Items.Add(Item($"고정 (두 상태 모두 #{cell.Current}, 문틀 같은 칸)", () => cell.Partner = cell.Current));
            menu.Items.Add(Item("비우기 (#0, 반대 상태에서 오브젝트 없음)", () => cell.Partner = 0));
            menu.IsOpen = true;
        }

        private static MenuItem ObjectItem(ClientAssets assets, int id, string header, DoorEditCell cell)
        {
            var item = Item(header, () => cell.Partner = id);
            var bitmap = assets == null ? null : Thumbnail.Object(assets, id);
            if (bitmap != null)
            {
                var image = new Image { Source = bitmap, Height = 40, Stretch = Stretch.Uniform, StretchDirection = StretchDirection.DownOnly };
                RenderOptions.SetBitmapScalingMode(image, BitmapScalingMode.NearestNeighbor);
                item.Icon = image;
            }
            return item;
        }

        private static MenuItem Item(string header, Action action)
        {
            var item = new MenuItem { Header = header };
            item.Click += (s, e) => action();
            return item;
        }

        private void OnFillSuggestions(object sender, RoutedEventArgs e)
        {
            foreach (var cell in _cells.Where(c => c.Fixed))
            {
                var partners = _editor.DoorPartners(cell.Current, CurrentIsClosed);
                if (partners.Count > 0)
                    cell.Partner = partners[0];
            }
        }

        private void OnFillPalette(object sender, RoutedEventArgs e)
        {
            for (int i = 0; i < _cells.Count; i++)
                _cells[i].Partner = _editor.SelectedObject + i;
        }

        private void OnApply(object sender, RoutedEventArgs e)
        {
            if (_editor.Document == null)
                return;

            var cells = _cells.Select(c => (c.OpenId, c.CloseId)).ToList();
            if (cells.SequenceEqual(_original))
            {
                Message.Foreground = Brushes.Orange;
                Message.Text = "바뀐 짝이 없습니다. 지금 인식되는 문을 그대로 둡니다.";
                return;
            }

            var modify = ModifyExisting.IsChecked == true ? _existing : null;
            var (model, found, saved) = _editor.ApplyDoorEdit(_x, _y, cells, modify);
            _applied = saved ? found : null;
            ToggleButton.IsEnabled = _applied != null;
            if (saved)
            {
                _original = cells;
                Message.Foreground = Brushes.LimeGreen;
                Message.Text = $"문 {model.Id} 적용: 이 위치에서 {(found.Opened ? "열린" : "닫힌")} 문으로 인식됩니다. door.xlsx 저장을 눌러야 파일에 남습니다.";
            }
            else if (found != null)
            {
                Message.Foreground = Brushes.Orange;
                Message.Text = $"적용하지 않았습니다: x {_x}에서 id가 더 작은 문 {found.Model.Id} (폭 {found.Width})이 먼저 인식됩니다 (서버도 id 순으로 찾음). "
                             + $"범위를 문 {found.Model.Id} 오른쪽부터 다시 잡거나, 문 {found.Model.Id}만 선택해 그 정의를 고치세요.";
            }
            else
            {
                Message.Foreground = Brushes.Orange;
                Message.Text = "적용하지 않았습니다: 이 정의로는 서버가 이 위치에서 문을 찾지 못합니다. 지금 맵의 오브젝트가 위 행(현재)과 다르거나 왼쪽 칸이 다른 문에 먼저 잡힙니다.";
            }
        }

        private void OnToggle(object sender, RoutedEventArgs e)
        {
            if (_applied == null || _editor.Document == null)
                return;

            _editor.ToggleDoor(_applied);
            _applied = _editor.Document.Doors.FirstOrDefault(d => d.Y == _y && d.X == _x && d.Model == _applied.Model);
            ToggleButton.IsEnabled = _applied != null;

            // The map now shows the other state, so the top row follows it.
            foreach (var cell in _cells)
                (cell.Current, cell.Partner) = (cell.Partner, cell.Current);
            if (CurrentIsClosed)
                CurrentOpened.IsChecked = true;
            else
                CurrentClosed.IsChecked = true;
        }

        private void OnClose(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
