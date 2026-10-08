using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Asset;
using MapEditor.Edit;

namespace MapEditor.Control
{
    public enum TemplateBrush
    {
        Draw,
        Erase,
        Pick,
    }

    /// <summary>
    /// Editable view of a template: cells without a tile show a checkerboard (left untouched when placed). Left
    /// button applies the brush on the active layer, right button erases it.
    /// </summary>
    public class TemplateCanvas : FrameworkElement
    {
        private static readonly Brush CheckLight = Freeze(new SolidColorBrush(Color.FromRgb(0x3a, 0x3a, 0x3a)));
        private static readonly Brush CheckDark = Freeze(new SolidColorBrush(Color.FromRgb(0x2a, 0x2a, 0x2a)));
        private static readonly Pen GridPen = Freeze(new Pen(new SolidColorBrush(Color.FromArgb(50, 255, 255, 255)), 1));
        private static readonly Pen BorderPen = Freeze(new Pen(new SolidColorBrush(Color.FromRgb(0xff, 0xa5, 0x30)), 1.5));
        private static readonly Pen HoverPen = Freeze(new Pen(Brushes.White, 1.5));
        private static readonly Brush ObjectMark = Freeze(new SolidColorBrush(Color.FromArgb(60, 0x00, 0x9b, 0xff)));

        private static T Freeze<T>(T freezable) where T : Freezable
        {
            freezable.Freeze();
            return freezable;
        }

        private BitmapSource _image;
        private int _above;
        private (int X, int Y)? _hover;
        private bool _painting;
        private bool _erasing;

        public MapTemplate Template { get; set; }
        public ClientAssets Assets { get; set; }
        public bool ObjectLayer { get; set; }
        public TemplateBrush Brush { get; set; }
        public ushort Value { get; set; } = 1;
        public double Zoom { get; set; } = 1;

        /// <summary>
        /// Cells changed; the window refreshes its counts.
        /// </summary>
        public event Action Changed;

        /// <summary>
        /// Eyedropper picked the value of the active layer.
        /// </summary>
        public event Action<ushort> Picked;

        public TemplateCanvas()
        {
            RenderOptions.SetBitmapScalingMode(this, BitmapScalingMode.NearestNeighbor);
        }

        private double Scale => ClientAssets.CellSize * Zoom;

        /// <summary>
        /// Redraws the template picture and the layout after any change of Template, Assets or Zoom.
        /// </summary>
        public void Refresh()
        {
            if (Template != null && Assets != null)
            {
                var drawn = Thumbnail.Template(Assets, Template);
                _image = drawn.Bitmap;
                _above = drawn.Above;
            }
            InvalidateMeasure();
            InvalidateVisual();
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (Template == null)
                return new Size();
            return new Size(Math.Max(1, Template.Width) * Scale + 2, (Math.Max(1, Template.Height) + _above) * Scale + 2);
        }

        private Rect CellRect(int x, int y)
        {
            return new Rect(1 + x * Scale, 1 + (y + _above) * Scale, Scale, Scale);
        }

        private (int X, int Y)? CellAt(Point point)
        {
            var x = (int)Math.Floor((point.X - 1) / Scale);
            var y = (int)Math.Floor((point.Y - 1) / Scale) - _above;
            return Template != null && x >= 0 && y >= 0 && x < Template.Width && y < Template.Height ? (x, y) : null;
        }

        protected override void OnRender(DrawingContext dc)
        {
            if (Template == null)
                return;

            var tiles = Template.Cells.Where(c => c.Tile != null).Select(c => (c.Dx, c.Dy)).ToHashSet();
            for (int y = 0; y < Template.Height; y++)
            {
                for (int x = 0; x < Template.Width; x++)
                {
                    if (tiles.Contains((x, y)))
                        continue;

                    var r = CellRect(x, y);
                    var half = Scale / 2;
                    dc.DrawRectangle(CheckDark, null, r);
                    dc.DrawRectangle(CheckLight, null, new Rect(r.X, r.Y, half, half));
                    dc.DrawRectangle(CheckLight, null, new Rect(r.X + half, r.Y + half, half, half));
                }
            }
            if (_image != null)
            {
                var picture = new DrawingGroup();
                RenderOptions.SetBitmapScalingMode(picture, Assets != null && Scale < Assets.CellPixels ? BitmapScalingMode.Linear : BitmapScalingMode.NearestNeighbor);
                using (var context = picture.Open())
                    context.DrawImage(_image, new Rect(1, 1, Math.Max(1, Template.Width) * Scale, (Math.Max(1, Template.Height) + _above) * Scale));
                dc.DrawDrawing(picture);
            }
            if (ObjectLayer)
            {
                foreach (var cell in Template.Cells.Where(c => c.Object is > 0))
                    dc.DrawRectangle(ObjectMark, null, CellRect(cell.Dx, cell.Dy));
            }

            var grid = new StreamGeometry();
            using (var context = grid.Open())
            {
                for (int x = 0; x <= Template.Width; x++)
                {
                    context.BeginFigure(new Point(1 + x * Scale, 1 + _above * Scale), false, false);
                    context.LineTo(new Point(1 + x * Scale, 1 + (_above + Template.Height) * Scale), true, false);
                }
                for (int y = 0; y <= Template.Height; y++)
                {
                    context.BeginFigure(new Point(1, 1 + (y + _above) * Scale), false, false);
                    context.LineTo(new Point(1 + Template.Width * Scale, 1 + (y + _above) * Scale), true, false);
                }
            }
            dc.DrawGeometry(null, GridPen, grid);
            dc.DrawRectangle(null, BorderPen, new Rect(1, 1 + _above * Scale, Template.Width * Scale, Template.Height * Scale));
            if (_hover is (int hx, int hy))
                dc.DrawRectangle(null, HoverPen, CellRect(hx, hy));
        }

        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            base.OnMouseDown(e);
            if (e.ChangedButton != MouseButton.Left && e.ChangedButton != MouseButton.Right)
                return;

            _erasing = e.ChangedButton == MouseButton.Right || Brush == TemplateBrush.Erase;
            if (e.ChangedButton == MouseButton.Left && Brush == TemplateBrush.Pick)
            {
                if (CellAt(e.GetPosition(this)) is (int px, int py))
                {
                    var cell = Template.Cells.FirstOrDefault(c => c.Dx == px && c.Dy == py);
                    var value = ObjectLayer ? cell.Object : cell.Tile;
                    if (value is ushort picked)
                        Picked?.Invoke(picked);
                }
                return;
            }
            _painting = true;
            CaptureMouse();
            PaintAt(e.GetPosition(this));
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            var cell = CellAt(e.GetPosition(this));
            if (cell != _hover)
            {
                _hover = cell;
                InvalidateVisual();
            }
            if (_painting)
                PaintAt(e.GetPosition(this));
        }

        protected override void OnMouseUp(MouseButtonEventArgs e)
        {
            base.OnMouseUp(e);
            _painting = false;
            ReleaseMouseCapture();
        }

        protected override void OnMouseLeave(MouseEventArgs e)
        {
            base.OnMouseLeave(e);
            _hover = null;
            InvalidateVisual();
        }

        private void PaintAt(Point point)
        {
            if (CellAt(point) is not (int x, int y))
                return;

            var index = Template.Cells.FindIndex(c => c.Dx == x && c.Dy == y);
            var cell = index >= 0 ? Template.Cells[index] : new TemplateCell { Dx = x, Dy = y };
            var before = cell;
            ushort? value = _erasing ? null : Value;
            if (ObjectLayer)
                cell.Object = value is 0 ? null : value;
            else
                cell.Tile = value;
            if (before.Tile == cell.Tile && before.Object == cell.Object)
                return;

            var cells = Template.Cells.ToList();
            if (index >= 0)
                cells.RemoveAt(index);
            if (cell.Tile != null || cell.Object != null)
                cells.Add(cell);
            Template.Cells = cells;
            Template.Revision++;
            Refresh();
            Changed?.Invoke();
        }
    }
}
