using System.Windows;
using System.Windows.Controls;
using MapEditor.Asset;
using MapEditor.Edit;
using MapEditor.ViewModel;

namespace MapEditor
{
    /// <summary>
    /// New map: id, name, size, base tile and the map whose map.xlsx settings are copied.
    /// Save as: id, name and sheet for a copy of an open tab; size, tiles and settings come from the tab's map.
    /// </summary>
    public partial class NewMapWindow : Window
    {
        private readonly MainWindowViewModel _editor;
        private readonly MapDocument _source;

        private NewMapWindow(MainWindowViewModel editor, MapDocument source)
        {
            InitializeComponent();
            _editor = editor;
            _source = source;
            var template = source?.Id ?? editor.Document?.Id ?? editor.Maps.FirstOrDefault()?.Id ?? 0;
            IdBox.Text = ((editor.Maps.Count == 0 ? 0 : editor.Maps.Max(m => m.Id)) + 1).ToString();
            NameBox.Text = source?.Name ?? "새 맵";
            TileBox.Text = editor.SelectedTile.ToString();
            SheetBox.ItemsSource = editor.MapSheets;
            TemplateBox.Text = template.ToString();
            SheetBox.SelectedItem = editor.MapSheet(template) ?? editor.MapSheets.FirstOrDefault();
            if (source != null)
            {
                Title = $"다른 이름으로 저장: {source.Id:000000} {source.Name}";
                CreateButton.Content = "저장";
                SizeLabel.Visibility = SizePanel.Visibility = Visibility.Collapsed;
                TileLabel.Visibility = TilePanel.Visibility = Visibility.Collapsed;
                PickTemplateButton.Visibility = Visibility.Collapsed;
                TemplateBox.IsReadOnly = true;
            }
            Loaded += (s, e) =>
            {
                var box = source == null ? NameBox : IdBox;
                box.Focus();
                box.SelectAll();
            };
        }

        /// <summary>
        /// Shows the dialog and creates the map in a new tab.
        /// </summary>
        public static async Task Show(MainWindowViewModel editor)
        {
            if (editor.CanOpen == false)
                return;

            var window = new NewMapWindow(editor, null) { Owner = Application.Current.MainWindow };
            if (window.ShowDialog() != true)
                return;

            try
            {
                await editor.CreateMap(int.Parse(window.IdBox.Text), window.NameBox.Text.Trim(), int.Parse(window.WidthBox.Text), int.Parse(window.HeightBox.Text),
                                       int.Parse(window.TileBox.Text), (string)window.SheetBox.SelectedItem, int.Parse(window.TemplateBox.Text));
            }
            catch (Exception e)
            {
                MessageBox.Show($"맵을 만들지 못했습니다.\n{e.Message}", "새 맵", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        /// <summary>
        /// Shows the dialog and saves the tab as a new map that takes the tab's place.
        /// </summary>
        public static async Task ShowSaveAs(MainWindowViewModel editor, MapDocument source)
        {
            if (editor.CanOpen == false || source == null)
                return;

            var window = new NewMapWindow(editor, source) { Owner = Application.Current.MainWindow };
            if (window.ShowDialog() != true)
                return;

            try
            {
                await editor.SaveMapAs(source, int.Parse(window.IdBox.Text), window.NameBox.Text.Trim(), (string)window.SheetBox.SelectedItem);
            }
            catch (Exception e)
            {
                MessageBox.Show($"다른 이름으로 저장하지 못했습니다.\n{e.Message}", "다른 이름으로 저장", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void OnTileChanged(object sender, TextChangedEventArgs e)
        {
            TilePreview.Source = _editor?.Assets != null && int.TryParse(TileBox.Text, out var tile) ? Thumbnail.Tile(_editor.Assets, tile) : null;
        }

        private void OnTemplateChanged(object sender, TextChangedEventArgs e)
        {
            if (_editor == null)
                return;

            var entry = int.TryParse(TemplateBox.Text, out var id) ? _editor.MapChoices.FirstOrDefault(m => m.Id == id) : null;
            TemplateName.Text = entry == null ? "map.xlsx에 없는 맵입니다." : $"{entry.Name}: bgm, effect, host, option 등 map.xlsx 설정을 복사합니다.";
            var sheet = entry == null ? null : _editor.MapSheet(entry.Id);
            if (sheet != null)
                SheetBox.SelectedItem = sheet;
        }

        private void OnPickTemplate(object sender, RoutedEventArgs e)
        {
            var entry = PickerWindow.Show("설정 복사할 맵", _editor.MapChoices);
            if (entry != null)
                TemplateBox.Text = entry.Id.ToString();
        }

        private void OnCreate(object sender, RoutedEventArgs e)
        {
            string error = null;
            if (int.TryParse(IdBox.Text, out var id) == false || id <= 0 || id > 65535)
                error = "맵 번호는 1 ~ 65535 입니다.";
            else if (_editor.Maps.Any(m => m.Id == id) || _editor.MapChoices.Any(m => m.Id == id))
                error = $"맵 {id}은 이미 있습니다.";
            else if (string.IsNullOrWhiteSpace(NameBox.Text))
                error = "이름을 입력하세요.";
            else if (_source == null && (int.TryParse(WidthBox.Text, out var width) == false || int.TryParse(HeightBox.Text, out var height) == false || width < 8 || height < 8 || width > 255 || height > 255))
                error = "가로와 세로는 8 ~ 255 입니다.";
            else if (_source == null && (int.TryParse(TileBox.Text, out var tile) == false || tile < 0 || (_editor.Assets != null && tile >= _editor.Assets.TileCount)))
                error = $"타일은 0 ~ {(_editor.Assets?.TileCount ?? 1) - 1} 입니다.";
            else if (int.TryParse(TemplateBox.Text, out var template) == false || _editor.MapChoices.Any(m => m.Id == template) == false)
                error = _source == null ? "설정 복사할 맵이 map.xlsx에 없습니다." : "원본 맵이 map.xlsx에 없어 설정을 복사할 수 없습니다.";
            else if (SheetBox.SelectedItem == null)
                error = "시트를 고르세요.";

            if (error != null)
            {
                MessageBox.Show(this, error, Title, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            DialogResult = true;
        }
    }
}
