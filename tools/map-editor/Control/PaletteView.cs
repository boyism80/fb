using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using MapEditor.Asset;
using MapEditor.Edit;
using MapEditor.ViewModel;

namespace MapEditor.Control
{
    public enum PaletteKind
    {
        Tile,
        Object,
    }

    /// <summary>
    /// Virtualized thumbnail grid of every tile or object id; only visible rows are drawn.
    /// </summary>
    public class PaletteView : FrameworkElement, IScrollInfo
    {
        public static readonly DependencyProperty EditorProperty = DependencyProperty.Register(
            nameof(Editor), typeof(MainWindowViewModel), typeof(PaletteView), new PropertyMetadata(null, OnEditorChanged));

        public static readonly DependencyProperty KindProperty = DependencyProperty.Register(
            nameof(Kind), typeof(PaletteKind), typeof(PaletteView), new PropertyMetadata(PaletteKind.Tile));

        public MainWindowViewModel Editor
        {
            get => (MainWindowViewModel)GetValue(EditorProperty);
            set => SetValue(EditorProperty, value);
        }

        public PaletteKind Kind
        {
            get => (PaletteKind)GetValue(KindProperty);
            set => SetValue(KindProperty, value);
        }

        private double _offset;
        private Size _viewport;
        private int _hover = -1;

        private static readonly Pen SelectedPen = new Pen(new SolidColorBrush(Color.FromRgb(0x00, 0x9b, 0xff)), 2);
        private static readonly Pen HoverPen = new Pen(Brushes.White, 1);
        private static readonly Brush CellBrush = new SolidColorBrush(Color.FromRgb(0x0e, 0x0e, 0x0e));
        private static readonly Typeface LabelFace = new Typeface("Consolas");

        public PaletteView()
        {
            ClipToBounds = true;
            RenderOptions.SetBitmapScalingMode(this, BitmapScalingMode.NearestNeighbor);
        }

        private static void OnEditorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var view = (PaletteView)d;
            if (e.OldValue is MainWindowViewModel old)
                old.PropertyChanged -= view.OnEditorPropertyChanged;
            if (e.NewValue is MainWindowViewModel editor)
                editor.PropertyChanged += view.OnEditorPropertyChanged;
        }

        private void OnEditorPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(MainWindowViewModel.Assets))
            {
                _offset = 0;
                UpdateScroll();
                InvalidateVisual();
            }
            else if ((Kind == PaletteKind.Tile && e.PropertyName == nameof(MainWindowViewModel.SelectedTile)) ||
                     (Kind == PaletteKind.Object && e.PropertyName == nameof(MainWindowViewModel.SelectedObject)))
            {
                ScrollTo(Selected);
                InvalidateVisual();
            }
        }

        private double CellSize => Kind == PaletteKind.Tile ? 40 : 56;
        private int Count => Editor?.Assets == null ? 0 : Kind == PaletteKind.Tile ? Editor.Assets.TileCount - 1 : Editor.Assets.Objects.Count;
        private int Columns => Math.Max(1, (int)(_viewport.Width / CellSize));
        private int Rows => (Count + Columns - 1) / Columns;
        private int Selected => Editor == null ? 0 : Kind == PaletteKind.Tile ? Editor.SelectedTile : Editor.SelectedObject;

        /// <summary>
        /// Ids start at 1 in both palettes (0 is "nothing").
        /// </summary>
        private int IdAt(Point point)
        {
            var column = (int)(point.X / CellSize);
            var row = (int)((point.Y + _offset) / CellSize);
            if (column >= Columns)
                return -1;

            var id = row * Columns + column + 1;
            return id <= Count ? id : -1;
        }

        public void ScrollTo(int id)
        {
            if (id <= 0)
                return;

            var top = (id - 1) / Columns * CellSize;
            if (top < _offset || top + CellSize > _offset + _viewport.Height)
            {
                _offset = top - _viewport.Height / 2;
                UpdateScroll();
            }
        }

        private void UpdateScroll()
        {
            _offset = Math.Clamp(_offset, 0, Math.Max(0, Rows * CellSize - _viewport.Height));
            ScrollOwner?.InvalidateScrollInfo();
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            return new Size(double.IsInfinity(availableSize.Width) ? 0 : availableSize.Width,
                            double.IsInfinity(availableSize.Height) ? 0 : availableSize.Height);
        }

        protected override Size ArrangeOverride(Size finalSize)
        {
            _viewport = finalSize;
            UpdateScroll();
            return finalSize;
        }

        protected override void OnRender(DrawingContext dc)
        {
            dc.DrawRectangle(Brushes.Transparent, null, new Rect(0, 0, ActualWidth, ActualHeight));
            var assets = Editor?.Assets;
            if (assets == null)
                return;

            var size = CellSize;
            var firstRow = (int)(_offset / size);
            var lastRow = (int)((_offset + _viewport.Height) / size);
            for (int row = firstRow; row <= lastRow; row++)
            {
                for (int column = 0; column < Columns; column++)
                {
                    var id = row * Columns + column + 1;
                    if (id > Count)
                        break;

                    var rect = new Rect(column * size + 1, row * size - _offset + 1, size - 2, size - 2);
                    dc.DrawRectangle(CellBrush, null, rect);
                    var image = Kind == PaletteKind.Tile ? Thumbnail.Tile(assets, id) : Thumbnail.Object(assets, id);
                    if (image != null)
                    {
                        var scale = Math.Min(1.5, Math.Min((rect.Width - 4) / image.Width, (rect.Height - 4) / image.Height));
                        var w = image.Width * scale;
                        var h = image.Height * scale;
                        dc.DrawImage(image, new Rect(rect.X + (rect.Width - w) / 2, rect.Y + (rect.Height - h) / 2, w, h));
                    }

                    if (id == Selected)
                        dc.DrawRectangle(null, SelectedPen, rect);
                    else if (id == _hover)
                        dc.DrawRectangle(null, HoverPen, rect);
                }
            }

            if (_hover > 0)
            {
                var text = new FormattedText($"{Kind} {_hover}", CultureInfo.InvariantCulture, FlowDirection.LeftToRight, LabelFace, 11,
                                             Brushes.White, VisualTreeHelper.GetDpi(this).PixelsPerDip);
                dc.DrawRectangle(new SolidColorBrush(Color.FromArgb(200, 0, 0, 0)), null, new Rect(0, 0, text.Width + 6, text.Height + 2));
                dc.DrawText(text, new Point(3, 1));
            }
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            var id = IdAt(e.GetPosition(this));
            if (id != _hover)
            {
                _hover = id;
                InvalidateVisual();
            }
        }

        protected override void OnMouseLeave(MouseEventArgs e)
        {
            base.OnMouseLeave(e);
            _hover = -1;
            InvalidateVisual();
        }

        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            base.OnMouseDown(e);
            var id = IdAt(e.GetPosition(this));
            if (id <= 0 || Editor == null)
                return;

            if (Kind == PaletteKind.Tile)
            {
                Editor.SelectedTile = id;
                Editor.Layer = EditLayer.Tile;
            }
            else
            {
                Editor.SelectedObject = id;
                Editor.Layer = EditLayer.Object;
            }
            if (Editor.Tool != EditTool.Rect && Editor.Tool != EditTool.Fill)
                Editor.Tool = EditTool.Brush;
            InvalidateVisual();
        }

        protected override void OnMouseWheel(MouseWheelEventArgs e)
        {
            base.OnMouseWheel(e);
            SetVerticalOffset(_offset - e.Delta);
            e.Handled = true;
        }

        public bool CanHorizontallyScroll { get; set; }
        public bool CanVerticallyScroll { get; set; }
        public double ExtentWidth => _viewport.Width;
        public double ExtentHeight => Math.Max(Rows * CellSize, _viewport.Height);
        public double ViewportWidth => _viewport.Width;
        public double ViewportHeight => _viewport.Height;
        public double HorizontalOffset => 0;
        public double VerticalOffset => _offset;
        public ScrollViewer ScrollOwner { get; set; }

        public void LineUp() => SetVerticalOffset(_offset - CellSize);
        public void LineDown() => SetVerticalOffset(_offset + CellSize);
        public void LineLeft() { }
        public void LineRight() { }
        public void PageUp() => SetVerticalOffset(_offset - _viewport.Height);
        public void PageDown() => SetVerticalOffset(_offset + _viewport.Height);
        public void PageLeft() { }
        public void PageRight() { }
        public void MouseWheelUp() => LineUp();
        public void MouseWheelDown() => LineDown();
        public void MouseWheelLeft() { }
        public void MouseWheelRight() { }
        public void SetHorizontalOffset(double offset) { }

        public void SetVerticalOffset(double offset)
        {
            _offset = offset;
            UpdateScroll();
            InvalidateVisual();
        }

        public Rect MakeVisible(Visual visual, Rect rectangle)
        {
            return rectangle;
        }
    }
}
