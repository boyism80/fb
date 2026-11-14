using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Asset;
using MapEditor.Edit;
using MapEditor.Format;
using MapEditor.ViewModel;

namespace MapEditor
{
    public partial class WarpAlignWindow : Window
    {
        private readonly MainWindowViewModel _editor;
        private readonly List<WarpAlignProposal> _all = new List<WarpAlignProposal>();
        private readonly ObservableCollection<WarpAlignProposal> _shown = new ObservableCollection<WarpAlignProposal>();
        private CancellationTokenSource _cancel;
        private int _bulk;

        public WarpAlignWindow(MainWindowViewModel editor)
        {
            _editor = editor;
            InitializeComponent();
            ProposalGrid.ItemsSource = _shown;
        }

        private async void OnStart(object sender, RoutedEventArgs e)
        {
            _cancel?.Cancel();
            var cancel = new CancellationTokenSource();
            _cancel = cancel;
            var token = cancel.Token;
            StartButton.IsEnabled = false;
            CancelButton.IsEnabled = true;
            ProgressBar.Value = 0;
            ProgressBar.IsIndeterminate = false;
            var progress = new Progress<(double Done, string Text)>(p =>
            {
                ProgressBar.Value = p.Done;
                ProgressText.Text = p.Text;
            });
            try
            {
                var found = await _editor.SearchWarpAlign(progress, token);
                foreach (var row in _all)
                {
                    row.PropertyChanged -= OnRowChanged;
                }
                _all.Clear();
                _all.AddRange(found);
                foreach (var row in _all)
                {
                    row.PropertyChanged += OnRowChanged;
                }
                ApplyFilter();
                ProgressText.Text = $"제안 {_all.Count}곳. 체크한 뒤 적용합니다. 막힌 칸이나 다른 워프가 있는 곳은 적용에서 빠집니다.";
            }
            catch (OperationCanceledException)
            {
                ProgressText.Text = "검색을 중지했습니다.";
            }
            catch (Exception ex)
            {
                ProgressText.Text = ex.Message;
            }
            finally
            {
                if (ReferenceEquals(_cancel, cancel))
                {
                    StartButton.IsEnabled = true;
                    CancelButton.IsEnabled = false;
                }
            }
        }

        private void OnCancel(object sender, RoutedEventArgs e)
        {
            _cancel?.Cancel();
        }

        private void OnFilterChanged(object sender, TextChangedEventArgs e)
        {
            ApplyFilter();
        }

        private void ApplyFilter()
        {
            var query = FilterBox.Text?.Trim() ?? "";
            _shown.Clear();
            foreach (var row in _all)
            {
                if (query.Length == 0 || Matches(row, query))
                {
                    _shown.Add(row);
                }
            }
            RefreshSelectAll();
            if (ProposalGrid != null && ProposalGrid.SelectedItem is WarpAlignProposal == false && _shown.Count > 0)
            {
                ProposalGrid.SelectedIndex = 0;
            }
        }

        private static bool Matches(WarpAlignProposal row, string query)
        {
            return row.MapLabel.Contains(query, StringComparison.OrdinalIgnoreCase)
                || row.Anchor.Contains(query, StringComparison.OrdinalIgnoreCase)
                || row.Cells.Contains(query, StringComparison.OrdinalIgnoreCase)
                || row.ProposedCells.Contains(query, StringComparison.OrdinalIgnoreCase)
                || row.Action.Contains(query, StringComparison.OrdinalIgnoreCase)
                || row.Note.Contains(query, StringComparison.OrdinalIgnoreCase)
                || row.Destination.Contains(query, StringComparison.OrdinalIgnoreCase)
                || row.Place.Contains(query, StringComparison.OrdinalIgnoreCase);
        }

        private void OnRowChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(WarpAlignProposal.Checked))
            {
                RefreshSelectAll();
            }
        }

        private void OnSelectAll(object sender, RoutedEventArgs e)
        {
            if (_bulk > 0)
            {
                return;
            }

            _bulk++;
            var on = SelectAllBox.IsChecked == true;
            foreach (var row in _shown)
            {
                row.Checked = on;
            }
            _bulk--;
        }

        private void RefreshSelectAll()
        {
            if (_bulk > 0 || SelectAllBox == null)
            {
                return;
            }

            _bulk++;
            SelectAllBox.IsChecked = _shown.Count > 0 && _shown.All(row => row.Checked);
            _bulk--;
        }

        private void OnSelected(object sender, SelectionChangedEventArgs e)
        {
            if (ProposalGrid.SelectedItem is WarpAlignProposal row)
            {
                ShowPreview(row);
            }
        }

        private void OnGridKey(object sender, KeyEventArgs e)
        {
            if (e.Key != Key.Space || Keyboard.Modifiers != ModifierKeys.None || ProposalGrid.SelectedItem is WarpAlignProposal row == false)
            {
                return;
            }

            row.Checked = !row.Checked;
            e.Handled = true;
        }

        private void OnDoubleClick(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            if (ProposalGrid.SelectedItem is WarpAlignProposal row)
            {
                _ = _editor.FocusWarpAlign(row);
            }
        }

        private void OnOpenMap(object sender, RoutedEventArgs e)
        {
            if (ProposalGrid.SelectedItem is WarpAlignProposal row)
            {
                _ = _editor.FocusWarpAlign(row);
            }
        }

        private async void OnApply(object sender, RoutedEventArgs e)
        {
            var chosen = _all.Where(row => row.Checked).ToList();
            if (chosen.Count == 0)
            {
                ProgressText.Text = "적용할 항목을 체크하세요.";
                return;
            }

            var blocked = chosen.Count(row => row.Blocked);
            var text = blocked == 0
                ? $"체크한 {chosen.Count}곳을 워프 시트에 반영합니다."
                : $"체크한 {chosen.Count}곳 중 막혀 있거나 다른 워프가 있는 {blocked}곳은 빼고 반영합니다.";
            if (MessageBox.Show(this, text, "워프 위치 통일", MessageBoxButton.OKCancel, MessageBoxImage.Question) != MessageBoxResult.OK)
            {
                return;
            }

            try
            {
                var (applied, skipped, message) = await _editor.ApplyWarpAlign(chosen);
                foreach (var row in applied)
                {
                    row.PropertyChanged -= OnRowChanged;
                    _all.Remove(row);
                }
                ApplyFilter();
                ProgressText.Text = message + (skipped > 0 ? " 건너뛴 항목은 목록에 남아 있습니다." : "");
            }
            catch (Exception ex)
            {
                ProgressText.Text = ex.Message;
            }
        }

        private void ShowPreview(WarpAlignProposal row)
        {
            PreviewCaption.Text = $"{row.MapLabel}  {row.Place}  {row.Offset}\n지금 워프 {row.Cells}  →  {row.ProposedCells}\n지금 착지 {row.CurrentLandingText}  →  {row.ProposedLanding}";
            PreviewNote.Text = string.IsNullOrEmpty(row.Note) ? row.Action : $"{row.Action}  {row.Note}";
            var assets = _editor.Assets;
            if (assets?.Objects == null)
            {
                PreviewImage.Source = null;
                return;
            }

            var (map, blocks) = _editor.WarpAlignView(row.MapId);
            if (map == null)
            {
                PreviewImage.Source = null;
                PreviewNote.Text = "맵 파일이 없습니다. " + PreviewNote.Text;
                return;
            }

            PreviewImage.Source = Draw(assets, map, blocks, row);
        }

        private static BitmapSource Draw(ClientAssets assets, ServerMap map, BlockFile blocks, WarpAlignProposal row)
        {
            var points = row.Current.Concat(row.Proposed).Concat(row.CurrentLanding).Concat(row.NewLanding).ToList();
            if (points.Count == 0)
            {
                return null;
            }

            var pad = 4;
            var topPad = pad + Math.Min(assets.Objects.MaxHeight, 8);
            var minX = Math.Max(0, points.Min(p => p.X) - pad);
            var minY = Math.Max(0, points.Min(p => p.Y) - topPad);
            var maxX = Math.Min(map.Width, points.Max(p => p.X) + pad + 1);
            var maxY = Math.Min(map.Height, points.Max(p => p.Y) + pad + 1);
            var width = Math.Max(1, maxX - minX);
            var height = Math.Max(1, maxY - minY);
            var cell = assets.CellPixels;
            var pw = width * cell;
            var ph = height * cell;
            var pixels = new uint[pw * ph];
            Array.Fill(pixels, 0xFF000000u);
            for (int cy = minY; cy < minY + height; cy++)
            {
                for (int cx = minX; cx < minX + width; cx++)
                {
                    if (map.Contains(cx, cy) && map.Tiles[cy * map.Width + cx] != 0)
                    {
                        assets.DrawTile(pixels, pw, ph, (cx - minX) * cell, (cy - minY) * cell, map.Tiles[cy * map.Width + cx]);
                    }
                }
            }
            for (int cy = minY; cy < Math.Min(map.Height, minY + height + assets.Objects.MaxHeight); cy++)
            {
                for (int cx = minX - 1; cx <= minX + width; cx++)
                {
                    if (map.Contains(cx, cy))
                    {
                        assets.DrawObject(pixels, pw, ph, (cx - minX) * cell, (cy - minY) * cell, map.Objects[cy * map.Width + cx]);
                    }
                }
            }
            for (int cy = minY; cy < minY + height; cy++)
            {
                for (int cx = minX; cx < minX + width; cx++)
                {
                    if (map.Contains(cx, cy) == false || blocks.Contains(cx, cy) == false)
                    {
                        continue;
                    }

                    for (int py = 0; py < cell; py++)
                    {
                        for (int px = 0; px < cell; px++)
                        {
                            var i = ((cy - minY) * cell + py) * pw + (cx - minX) * cell + px;
                            var color = pixels[i];
                            var r = (((color >> 16) & 0xFF) + 255) / 2;
                            var g = ((color >> 8) & 0xFF) / 2;
                            var b = (color & 0xFF) / 2;
                            pixels[i] = 0xFF000000u | (r << 16) | (g << 8) | b;
                        }
                    }
                }
            }

            Box(pixels, pw, ph, cell, minX, minY, row.Current, 0, 0xFFFF00FFu);
            Box(pixels, pw, ph, cell, minX, minY, row.CurrentLanding, 3, 0xFFFF8800u);
            Box(pixels, pw, ph, cell, minX, minY, row.Proposed, 6, 0xFFFFFF00u);
            Box(pixels, pw, ph, cell, minX, minY, row.NewLanding, 9, 0xFF00FF66u);
            var bitmap = BitmapSource.Create(pw, ph, 96, 96, PixelFormats.Pbgra32, null, pixels, pw * 4);
            bitmap.Freeze();
            return bitmap;
        }

        private static void Box(uint[] pixels, int width, int height, int cell, int originX, int originY, List<(int X, int Y)> cells, int inset, uint color)
        {
            foreach (var (x, y) in cells)
            {
                var left = (x - originX) * cell + inset;
                var top = (y - originY) * cell + inset;
                var right = (x - originX + 1) * cell - 1 - inset;
                var bottom = (y - originY + 1) * cell - 1 - inset;
                if (right - left < 1 || bottom - top < 1)
                {
                    continue;
                }

                for (int py = Math.Max(0, top); py <= Math.Min(height - 1, bottom); py++)
                {
                    for (int px = Math.Max(0, left); px <= Math.Min(width - 1, right); px++)
                    {
                        if (px - left < 2 || right - px < 2 || py - top < 2 || bottom - py < 2)
                        {
                            pixels[py * width + px] = color;
                        }
                    }
                }
            }
        }
    }
}
