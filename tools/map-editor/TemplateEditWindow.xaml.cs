using System.Windows;
using System.Windows.Controls;
using MapEditor.Asset;
using MapEditor.Control;
using MapEditor.Edit;

namespace MapEditor
{
    /// <summary>
    /// Edits a copy of a template: name, size, tiles and objects (each removable). Edit returns the edited copy,
    /// or null when cancelled.
    /// </summary>
    public partial class TemplateEditWindow : Window
    {
        private readonly MapTemplate _template;
        private readonly ClientAssets _assets;
        private ushort _tileValue = 1;
        private ushort _objectValue = 1;

        private TemplateEditWindow(MapTemplate template, ClientAssets assets)
        {
            _template = template;
            _assets = assets;
            InitializeComponent();
            NameBox.Text = template.Name;
            WidthBox.Text = template.Width.ToString();
            HeightBox.Text = template.Height.ToString();
            _tileValue = template.Cells.FirstOrDefault(c => c.Tile != null).Tile ?? 1;
            _objectValue = template.Cells.FirstOrDefault(c => c.Object != null).Object ?? 1;
            ValueBox.Text = _tileValue.ToString();

            Canvas.Template = template;
            Canvas.Assets = assets;
            Canvas.Zoom = ZoomSlider.Value;
            Canvas.Value = _tileValue;
            Canvas.Changed += UpdateInfo;
            Canvas.Picked += value =>
            {
                ValueBox.Text = value.ToString();
                DrawBrush.IsChecked = true;
            };
            Canvas.Refresh();
            UpdateInfo();
            Loaded += (s, e) =>
            {
                NameBox.Focus();
                NameBox.SelectAll();
            };
        }

        public static MapTemplate Edit(MapTemplate template, ClientAssets assets, string title)
        {
            var window = new TemplateEditWindow(template, assets) { Owner = Application.Current.MainWindow, Title = title };
            return window.ShowDialog() == true ? template : null;
        }

        private void UpdateInfo()
        {
            Info.Text = $"{_template.Width}×{_template.Height}, 오브젝트 {_template.ObjectCount}칸, 타일 {_template.TileCount}칸 | " +
                        "좌클릭: 도구, 우클릭: 지우기 | 체크무늬 칸은 타일 없음 (배치할 때 원래 타일을 그대로 둠)";
        }

        private void Changed()
        {
            _template.Revision++;
            Canvas.Refresh();
            UpdateInfo();
        }

        private void OnLayerChanged(object sender, RoutedEventArgs e)
        {
            if (Canvas == null)
                return;

            Canvas.ObjectLayer = ObjectLayer.IsChecked == true;
            ValueBox.Text = (Canvas.ObjectLayer ? _objectValue : _tileValue).ToString();
            Canvas.InvalidateVisual();
        }

        private void OnBrushChanged(object sender, RoutedEventArgs e)
        {
            if (Canvas == null)
                return;

            Canvas.Brush = EraseBrush.IsChecked == true ? TemplateBrush.Erase : PickBrush.IsChecked == true ? TemplateBrush.Pick : TemplateBrush.Draw;
        }

        private void OnValueChanged(object sender, TextChangedEventArgs e)
        {
            if (Canvas == null || ushort.TryParse(ValueBox.Text, out var value) == false)
            {
                if (ValuePreview != null)
                    ValuePreview.Source = null;
                return;
            }

            Canvas.Value = value;
            if (Canvas.ObjectLayer)
                _objectValue = value;
            else
                _tileValue = value;
            ValuePreview.Source = Canvas.ObjectLayer ? Thumbnail.Object(_assets, value) : Thumbnail.Tile(_assets, value);
        }

        private void OnZoomChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (Canvas == null)
                return;

            Canvas.Zoom = ZoomSlider.Value;
            Canvas.Refresh();
        }

        private void OnResize(object sender, RoutedEventArgs e)
        {
            if (int.TryParse(WidthBox.Text, out var width) == false || int.TryParse(HeightBox.Text, out var height) == false || width < 1 || height < 1 || width > 128 || height > 128)
            {
                MessageBox.Show(this, "크기는 1 ~ 128 입니다.", Title, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            var dropped = _template.Cells.Count(c => c.Dx >= width || c.Dy >= height);
            if (dropped > 0 && MessageBox.Show(this, $"크기 밖으로 나가는 {dropped}칸이 지워집니다. 계속할까요?", Title, MessageBoxButton.YesNo, MessageBoxImage.Warning) != MessageBoxResult.Yes)
                return;

            _template.Width = width;
            _template.Height = height;
            _template.Cells = _template.Cells.Where(c => c.Dx < width && c.Dy < height).ToList();
            Changed();
        }

        private void OnTrim(object sender, RoutedEventArgs e)
        {
            if (_template.Cells.Count == 0)
                return;

            var left = _template.Cells.Min(c => c.Dx);
            var top = _template.Cells.Min(c => c.Dy);
            _template.Width = _template.Cells.Max(c => c.Dx) - left + 1;
            _template.Height = _template.Cells.Max(c => c.Dy) - top + 1;
            _template.Cells = _template.Cells.Select(c => new TemplateCell { Dx = c.Dx - left, Dy = c.Dy - top, Tile = c.Tile, Object = c.Object }).ToList();
            WidthBox.Text = _template.Width.ToString();
            HeightBox.Text = _template.Height.ToString();
            Changed();
        }

        private void OnClearLooseTiles(object sender, RoutedEventArgs e)
        {
            _template.Cells = _template.Cells.Where(c => c.Object != null).ToList();
            Changed();
        }

        private void OnClearTiles(object sender, RoutedEventArgs e)
        {
            _template.Cells = _template.Cells.Where(c => c.Object != null).Select(c => new TemplateCell { Dx = c.Dx, Dy = c.Dy, Object = c.Object }).ToList();
            Changed();
        }

        private void OnOk(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(NameBox.Text))
            {
                MessageBox.Show(this, "이름을 입력하세요.", Title, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            if (_template.Cells.Count == 0)
            {
                MessageBox.Show(this, "타일이나 오브젝트가 하나도 없습니다.", Title, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            _template.Name = NameBox.Text.Trim();
            DialogResult = true;
        }
    }
}
