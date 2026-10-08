using System.Windows;
using System.Windows.Controls;
using MapEditor.ViewModel;

namespace MapEditor
{
    /// <summary>
    /// Ctrl+G: a map (id or name) and a cell; opens the map when needed and centers the cell.
    /// </summary>
    public partial class GoToWindow : Window
    {
        private readonly MainWindowViewModel _editor;
        private MapEntry _entry;
        private int _x;
        private int _y;

        public GoToWindow(MainWindowViewModel editor)
        {
            InitializeComponent();
            _editor = editor;
            var current = editor.Maps.FirstOrDefault(m => m.Id == editor.Document?.Id);
            MapBox.Text = current?.Label ?? "";
            Loaded += (s, e) =>
            {
                if (current != null)
                    XBox.Focus();
                else
                    MapBox.Focus();
            };
        }

        public static async Task Show(Window owner, MainWindowViewModel editor)
        {
            var window = new GoToWindow(editor) { Owner = owner };
            if (window.ShowDialog() != true)
                return;

            var (entry, x, y) = (window._entry, window._x, window._y);
            if (editor.Document?.Id != entry.Id)
                await editor.OpenMap(entry);
            if (editor.Document?.Id != entry.Id)
                return;

            if (editor.Document.Map.Contains(x, y) == false)
            {
                editor.StatusText = $"({x}, {y})는 {entry.Label} 밖입니다 (크기 {editor.Document.Width}×{editor.Document.Height}).";
                return;
            }
            editor.SelectedMap = entry;
            editor.ChangeSelection(new[] { (x, y) }, SelectMode.Replace);
            editor.Jump(x, y);
        }

        private void OnSelectAll(object sender, RoutedEventArgs e)
        {
            ((TextBox)sender).SelectAll();
        }

        private void OnGo(object sender, RoutedEventArgs e)
        {
            var text = MapBox.Text.Trim();
            var number = new string(text.TakeWhile(char.IsDigit).ToArray());
            List<MapEntry> found;
            if (number.Length > 0 && (number.Length == text.Length || text[number.Length] == ' '))
                found = _editor.Maps.Where(m => m.Id == int.Parse(number)).ToList();
            else
            {
                found = _editor.Maps.Where(m => m.Name == text).ToList();
                if (found.Count == 0)
                    found = _editor.Maps.Where(m => m.Name.Contains(text, StringComparison.OrdinalIgnoreCase)).ToList();
            }

            if (text.Length == 0 || found.Count == 0)
            {
                Message.Text = $"맵 '{text}'을 찾지 못했습니다.";
                MapBox.Focus();
                return;
            }
            if (found.Count > 1)
            {
                Message.Text = $"'{text}'에 맞는 맵이 {found.Count}개입니다: {string.Join(", ", found.Take(6).Select(m => m.Label))}{(found.Count > 6 ? " ..." : "")}. 번호나 정확한 이름을 입력하세요.";
                MapBox.Focus();
                return;
            }

            // "108, 92" or "108 92" typed into the x box fills both.
            var parts = YBox.Text.Trim().Length == 0
                ? XBox.Text.Split(new[] { ',', ' ' }, StringSplitOptions.RemoveEmptyEntries)
                : new[] { XBox.Text, YBox.Text };
            if (parts.Length != 2 || int.TryParse(parts[0].Trim(), out var x) == false || int.TryParse(parts[1].Trim(), out var y) == false || x < 0 || y < 0)
            {
                Message.Text = "x, y는 0 이상의 정수로 입력하세요.";
                XBox.Focus();
                return;
            }

            (_entry, _x, _y) = (found[0], x, y);
            DialogResult = true;
        }
    }
}
