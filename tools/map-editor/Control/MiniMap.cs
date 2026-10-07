using System.ComponentModel;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Asset;
using MapEditor.Edit;
using MapEditor.ViewModel;

namespace MapEditor.Control
{
    /// <summary>
    /// Whole-map overview with the canvas view rectangle. Click or drag to scroll the canvas there.
    /// Maps smaller than the minimap are baked with the real tile and object graphics and scaled down smoothly.
    /// Larger maps use one pixel per cell (the tile's average color, covered by the average color of each object
    /// piece that reaches the cell); when such a bitmap still has to be enlarged it is drawn at a whole-number scale
    /// without smoothing, so cells stay sharp squares.
    /// </summary>
    public class MiniMap : FrameworkElement
    {
        private const double MaxSide = 200;

        /// <summary>
        /// Largest detailed bitmap in pixels; bigger maps fall back to one pixel per cell.
        /// </summary>
        private const long MaxDetailedPixels = 6_000_000;

        public static readonly DependencyProperty CanvasProperty = DependencyProperty.Register(
            nameof(Canvas), typeof(MapCanvas), typeof(MiniMap), new PropertyMetadata(null, OnCanvasChanged));

        public MapCanvas Canvas
        {
            get => (MapCanvas)GetValue(CanvasProperty);
            set => SetValue(CanvasProperty, value);
        }

        public static readonly DependencyProperty EditorProperty = DependencyProperty.Register(
            nameof(Editor), typeof(MainWindowViewModel), typeof(MiniMap), new PropertyMetadata(null, OnEditorChanged));

        public MainWindowViewModel Editor
        {
            get => (MainWindowViewModel)GetValue(EditorProperty);
            set => SetValue(EditorProperty, value);
        }

        private static readonly Pen ViewPen = Freeze(new Pen(new SolidColorBrush(Color.FromRgb(0xFF, 0x40, 0x40)), 1.5));
        private static readonly Pen BorderPen = Freeze(new Pen(new SolidColorBrush(Color.FromRgb(0x42, 0x45, 0x4E)), 1));

        private MainWindowViewModel _editor;
        private MapDocument _document;
        private ClientAssets _assets;
        private WriteableBitmap _bitmap;
        private bool _dirty = true;
        private readonly Dictionary<int, uint> _tileColors = new Dictionary<int, uint>();
        private readonly Dictionary<int, uint> _pieceColors = new Dictionary<int, uint>();

        public MiniMap()
        {
            RenderOptions.SetBitmapScalingMode(this, BitmapScalingMode.HighQuality);
            UseLayoutRounding = true;
            SnapsToDevicePixels = true;
            Cursor = Cursors.Hand;
        }

        private static Pen Freeze(Pen pen)
        {
            pen.Freeze();
            return pen;
        }

        private static void OnCanvasChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var minimap = (MiniMap)d;
            if (e.OldValue is MapCanvas old)
                old.ViewChanged -= minimap.InvalidateVisual;
            if (e.NewValue is MapCanvas canvas)
                canvas.ViewChanged += minimap.InvalidateVisual;
        }

        private static void OnEditorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var minimap = (MiniMap)d;
            if (minimap._editor != null)
            {
                minimap._editor.PropertyChanged -= minimap.OnEditorPropertyChanged;
                minimap._editor.RenderInvalidated -= minimap.Rebuild;
            }
            minimap._editor = e.NewValue as MainWindowViewModel;
            if (minimap._editor != null)
            {
                minimap._editor.PropertyChanged += minimap.OnEditorPropertyChanged;
                minimap._editor.RenderInvalidated += minimap.Rebuild;
            }
            minimap.Rebuild();
        }

        private void OnEditorPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(MainWindowViewModel.Document) && ReferenceEquals(_document, _editor.Document) == false)
                Rebuild();
            else if (e.PropertyName == nameof(MainWindowViewModel.Assets))
                Rebuild();
        }

        private void Rebuild()
        {
            if (_document != null)
                _document.CellsChanged -= OnCellsChanged;
            _document = _editor?.Document;
            if (_document != null)
                _document.CellsChanged += OnCellsChanged;

            if (ReferenceEquals(_assets, _editor?.Assets) == false)
            {
                _assets = _editor?.Assets;
                _tileColors.Clear();
                _pieceColors.Clear();
            }
            _dirty = true;
            InvalidateMeasure();
            InvalidateVisual();
        }

        private void OnCellsChanged(IReadOnlyList<(int X, int Y)> cells)
        {
            _dirty = true;
            InvalidateVisual();
        }

        /// <summary>
        /// Pixels per cell in the baked bitmap: the resource cell size for detailed maps, otherwise 1.
        /// </summary>
        private int BakeCellPixels(MapDocument doc)
        {
            if (_assets == null || Math.Max(doc.Width, doc.Height) >= MaxSide)
                return 1;

            var cell = _assets.CellPixels;
            return (long)doc.Width * doc.Height * cell * cell <= MaxDetailedPixels ? cell : 1;
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (_document == null)
                return new Size(0, 0);

            var scale = MaxSide / Math.Max(_document.Width, _document.Height);
            if (scale > 1 && BakeCellPixels(_document) == 1)
                scale = Math.Floor(scale);
            return new Size(Math.Round(_document.Width * scale), Math.Round(_document.Height * scale));
        }

        protected override void OnRender(DrawingContext dc)
        {
            var doc = _document;
            var canvas = Canvas;
            if (doc == null || canvas == null)
                return;

            var cell = BakeCellPixels(doc);
            if (_dirty || _bitmap == null || _bitmap.PixelWidth != doc.Width * cell || _bitmap.PixelHeight != doc.Height * cell)
            {
                _bitmap = cell == 1 ? Bake(doc) : BakeDetailed(doc, cell);
                _dirty = false;
            }

            var enlarged = ActualWidth > _bitmap.PixelWidth;
            var mode = enlarged ? BitmapScalingMode.NearestNeighbor : BitmapScalingMode.HighQuality;
            if (RenderOptions.GetBitmapScalingMode(this) != mode)
                RenderOptions.SetBitmapScalingMode(this, mode);

            var bounds = new Rect(0, 0, ActualWidth, ActualHeight);
            dc.DrawImage(_bitmap, bounds);
            dc.DrawRectangle(null, BorderPen, bounds);

            var view = canvas.VisibleCells;
            var sx = ActualWidth / doc.Width;
            var sy = ActualHeight / doc.Height;
            var rect = Rect.Intersect(new Rect(view.X * sx, view.Y * sy, view.Width * sx, view.Height * sy), bounds);
            if (rect.IsEmpty == false)
                dc.DrawRectangle(null, ViewPen, rect);
        }

        /// <summary>
        /// The map drawn like the canvas: cell pixels per cell, tiles then objects top to bottom. Tiles and objects
        /// missing in 5.50 stay black while ShowMissing550 applies.
        /// </summary>
        private WriteableBitmap BakeDetailed(MapDocument doc, int cell)
        {
            var width = doc.Width * cell;
            var height = doc.Height * cell;
            var pixels = new uint[width * height];
            Array.Fill(pixels, 0xFF000000u);
            var limit = _editor.Limit550;
            if (_editor.ShowTiles)
            {
                for (int y = 0; y < doc.Height; y++)
                {
                    for (int x = 0; x < doc.Width; x++)
                    {
                        var tile = doc.Map.Tiles[y * doc.Width + x];
                        if (limit == null || tile < limit.Value.Tiles)
                            _assets.DrawTile(pixels, width, height, x * cell, y * cell, tile);
                    }
                }
            }
            if (_editor.ShowObjects)
            {
                for (int y = 0; y < doc.Height; y++)
                {
                    for (int x = 0; x < doc.Width; x++)
                    {
                        var id = doc.Map.Objects[y * doc.Width + x];
                        if (id != 0 && (limit == null || id <= limit.Value.Objects))
                            _assets.DrawObject(pixels, width, height, x * cell, y * cell, id);
                    }
                }
            }

            var bitmap = new WriteableBitmap(width, height, 96, 96, PixelFormats.Pbgra32, null);
            bitmap.WritePixels(new Int32Rect(0, 0, width, height), pixels, width * 4, 0);
            bitmap.Freeze();
            return bitmap;
        }

        private WriteableBitmap Bake(MapDocument doc)
        {
            var pixels = new uint[doc.Width * doc.Height];
            Array.Fill(pixels, 0xFF000000u);
            if (_assets != null)
            {
                var limit = _editor.Limit550;
                if (_editor.ShowTiles)
                {
                    for (int i = 0; i < pixels.Length; i++)
                    {
                        if (limit == null || doc.Map.Tiles[i] < limit.Value.Tiles)
                            pixels[i] = TileColor(doc.Map.Tiles[i]);
                    }
                }
                if (_editor.ShowObjects)
                {
                    // Top to bottom like the canvas so lower objects cover the ones behind them.
                    for (int y = 0; y < doc.Height; y++)
                    {
                        for (int x = 0; x < doc.Width; x++)
                        {
                            var id = doc.Map.Objects[y * doc.Width + x];
                            var sobj = limit != null && id > limit.Value.Objects ? null : _assets.Objects.Find(id);
                            if (sobj == null)
                                continue;

                            for (int k = 0; k < sobj.Frames.Length && y - k >= 0; k++)
                            {
                                var color = PieceColor(sobj.Frames[k]);
                                if (color != 0)
                                    pixels[(y - k) * doc.Width + x] = color;
                            }
                        }
                    }
                }
            }

            var bitmap = new WriteableBitmap(doc.Width, doc.Height, 96, 96, PixelFormats.Pbgra32, null);
            bitmap.WritePixels(new Int32Rect(0, 0, doc.Width, doc.Height), pixels, doc.Width * 4, 0);
            bitmap.Freeze();
            return bitmap;
        }

        private uint TileColor(int tile)
        {
            if (_tileColors.TryGetValue(tile, out var color) == false)
            {
                color = Average(_assets.TileFrame(tile), 1);
                if (color == 0)
                    color = 0xFF000000u;
                _tileColors[tile] = color;
            }
            return color;
        }

        /// <summary>
        /// Average color of an object piece, or 0 when it covers less than a third of a cell (thin poles, edges).
        /// </summary>
        private uint PieceColor(int frame)
        {
            if (_pieceColors.TryGetValue(frame, out var color) == false)
            {
                color = Average(_assets.ObjectFrame(frame), _assets.CellPixels * _assets.CellPixels / 3);
                _pieceColors[frame] = color;
            }
            return color;
        }

        private static uint Average(FrameImage frame, int minimumPixels)
        {
            if (frame == null)
                return 0;

            long r = 0, g = 0, b = 0;
            var count = 0;
            foreach (var pixel in frame.Pixels)
            {
                if (pixel == 0)
                    continue;

                r += (pixel >> 16) & 0xFF;
                g += (pixel >> 8) & 0xFF;
                b += pixel & 0xFF;
                count++;
            }
            if (count < minimumPixels)
                return 0;

            return 0xFF000000u | ((uint)(r / count) << 16) | ((uint)(g / count) << 8) | (uint)(b / count);
        }

        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            CaptureMouse();
            Scroll(e.GetPosition(this));
            e.Handled = true;
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (IsMouseCaptured)
                Scroll(e.GetPosition(this));
        }

        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
        {
            ReleaseMouseCapture();
        }

        private void Scroll(Point point)
        {
            if (_document == null || Canvas == null || ActualWidth <= 0 || ActualHeight <= 0)
                return;

            Canvas.CenterOn(point.X / ActualWidth * _document.Width, point.Y / ActualHeight * _document.Height);
        }
    }
}
