using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Asset;
using MapEditor.Edit;
using MapEditor.Table;
using MapEditor.ViewModel;

namespace MapEditor.Control
{
    /// <summary>
    /// Map view hosted in a ScrollViewer (CanContentScroll=True). Tiles and objects are baked into 32x32-cell chunk
    /// bitmaps; overlays are drawn each frame. A map smaller than the viewport is centered on that axis, so screen →
    /// cell conversion always goes through Origin.
    /// </summary>
    public class MapCanvas : FrameworkElement, IScrollInfo
    {
        private const int ChunkCells = 32;
        private const int Cell = ClientAssets.CellSize;
        private const int FeetY = 20;

        private enum DragKind
        {
            None,
            RubberBand,
            Move,
            ResizeMob,
            RectFill,
            MobArea,
            ContextArea,
        }

        public static readonly DependencyProperty EditorProperty = DependencyProperty.Register(
            nameof(Editor), typeof(MainWindowViewModel), typeof(MapCanvas), new PropertyMetadata(null, OnEditorChanged));

        public MainWindowViewModel Editor
        {
            get => (MainWindowViewModel)GetValue(EditorProperty);
            set => SetValue(EditorProperty, value);
        }

        private readonly Dictionary<(int, int), WriteableBitmap> _chunks = new Dictionary<(int, int), WriteableBitmap>();
        private MapDocument _document;
        private double _zoom = 1.0;
        private Vector _offset;
        private Size _viewport;
        private (int X, int Y)? _hover;
        private DragKind _drag;
        private (int X, int Y) _dragStart;
        private SelectMode _dragMode;
        private bool _toggleClick;
        private Int32Rect? _contextArea;
        private bool _altClicked;
        private MobSpawn _resizeMob;
        private (int X, int Y) _resizeCorner;
        private (int X, int Y) _lastStroke;
        private bool _stroking;
        private bool _spaceDown;
        private Point? _panStart;
        private Vector _panOffset;

        private static readonly Brush BackgroundBrush = Freeze(new SolidColorBrush(Color.FromRgb(0x1e, 0x1e, 0x1e)));
        private static readonly Brush MapBackBrush = Freeze(new SolidColorBrush(Colors.Black));
        private static readonly Brush BlockBrush = Freeze(new SolidColorBrush(Color.FromArgb(110, 255, 40, 40)));
        private static readonly Brush CollisionBrush = Freeze(new SolidColorBrush(Color.FromArgb(90, 255, 160, 0)));
        private static readonly Brush EdgeBrush = Freeze(new SolidColorBrush(Color.FromArgb(230, 255, 160, 0)));
        private static readonly Brush SelectionBrush = Freeze(new SolidColorBrush(Color.FromArgb(90, 0, 155, 255)));
        private static readonly Brush ContextAreaBrush = Freeze(new SolidColorBrush(Color.FromArgb(60, 0x00, 0x9b, 0xff)));
        private static readonly Pen ContextAreaPen = Freeze(new Pen(new SolidColorBrush(Color.FromRgb(0x00, 0x9b, 0xff)), 2));
        private static readonly Brush GhostBrush = Freeze(new SolidColorBrush(Color.FromArgb(70, 255, 255, 255)));
        private static readonly Brush LabelBackBrush = Freeze(new SolidColorBrush(Color.FromArgb(160, 0, 0, 0)));
        private static readonly Pen MapBorderPen = Freeze(new Pen(new SolidColorBrush(Color.FromRgb(0x42, 0x45, 0x4e)), 1));
        private static readonly Pen GridPen = Freeze(new Pen(new SolidColorBrush(Color.FromArgb(60, 255, 255, 255)), 1));
        private static readonly Pen HoverPen = Freeze(new Pen(Brushes.White, 1.5));
        private static readonly Pen DragPen = Freeze(new Pen(new SolidColorBrush(Color.FromRgb(0x00, 0x9b, 0xff)), 1.5) { DashStyle = DashStyles.Dash });
        private static readonly Pen GhostPen = Freeze(new Pen(Brushes.White, 1.5) { DashStyle = DashStyles.Dash });
        private static readonly Pen DoorOpenPen = Freeze(new Pen(Brushes.LimeGreen, 2));
        private static readonly Pen DoorClosePen = Freeze(new Pen(Brushes.Gold, 2));
        private static readonly Pen SelectedPen = Freeze(new Pen(Brushes.White, 3));
        private static readonly Pen MobPen = Freeze(new Pen(new SolidColorBrush(Color.FromRgb(0xc0, 0x60, 0xff)), 1.5));
        private static readonly Brush MobFill = Freeze(new SolidColorBrush(Color.FromArgb(40, 0xc0, 0x60, 0xff)));
        private static readonly Brush MobLabelBrush = Freeze(new SolidColorBrush(Color.FromRgb(0xe0, 0xb0, 0xff)));
        private static readonly Brush NpcBrush = Freeze(new SolidColorBrush(Color.FromRgb(0xff, 0x40, 0xc0)));
        private static readonly Brush WarpBrush = Freeze(new SolidColorBrush(Color.FromArgb(160, 0x00, 0xe0, 0xe0)));
        private static readonly Typeface LabelFace = new Typeface("Malgun Gothic");

        private static T Freeze<T>(T freezable) where T : Freezable
        {
            freezable.Freeze();
            return freezable;
        }

        public MapCanvas()
        {
            Focusable = true;
            ClipToBounds = true;
            RenderOptions.SetBitmapScalingMode(this, BitmapScalingMode.NearestNeighbor);
        }

        private static void OnEditorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var canvas = (MapCanvas)d;
            if (e.OldValue is MainWindowViewModel old)
            {
                old.RenderInvalidated -= canvas.OnRenderInvalidated;
                old.OverlayInvalidated -= canvas.InvalidateVisual;
                old.FocusRequested -= canvas.Focus;
                old.CenterRequested -= canvas.CenterOn;
                old.PropertyChanged -= canvas.OnEditorPropertyChanged;
            }
            if (e.NewValue is MainWindowViewModel editor)
            {
                editor.RenderInvalidated += canvas.OnRenderInvalidated;
                editor.OverlayInvalidated += canvas.InvalidateVisual;
                editor.FocusRequested += canvas.Focus;
                editor.CenterRequested += canvas.CenterOn;
                editor.PropertyChanged += canvas.OnEditorPropertyChanged;
            }
        }

        private void OnEditorPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(MainWindowViewModel.Zoom))
            {
                // ZoomAt already applied zooms that came from the wheel; only toolbar/menu zooms land here.
                if (Math.Abs(Editor.Zoom - _zoom) > 0.0001)
                    ZoomAt(Editor.Zoom, new Point(_viewport.Width / 2, _viewport.Height / 2));
            }
            else if (e.PropertyName == nameof(MainWindowViewModel.Document))
            {
                if (ReferenceEquals(_document, Editor.Document))
                {
                    InvalidateVisual();
                    return;
                }

                if (_document != null)
                    _document.CellsChanged -= OnCellsChanged;
                _document = Editor.Document;
                if (_document != null)
                    _document.CellsChanged += OnCellsChanged;

                _chunks.Clear();
                _offset = new Vector();
                _drag = DragKind.None;
                UpdateScroll();
                InvalidateVisual();
            }
            else if (e.PropertyName == nameof(MainWindowViewModel.Tool) || e.PropertyName == nameof(MainWindowViewModel.SelectedEntity))
            {
                InvalidateVisual();
            }
        }

        private void OnRenderInvalidated()
        {
            _chunks.Clear();
            InvalidateVisual();
        }

        private void OnCellsChanged(IReadOnlyList<(int X, int Y)> cells)
        {
            // An object drawn at row y covers rows above it, so chunks above a changed cell are dirty too.
            var height = Math.Max(1, Editor.Assets?.Objects.MaxHeight ?? 1);
            var dirty = new HashSet<(int, int)>();
            foreach (var (x, y) in cells)
            {
                for (int k = 0; k < height; k++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        var cx = x + dx;
                        var cy = y - k;
                        if (cx >= 0 && cy >= 0)
                            dirty.Add((cx / ChunkCells, cy / ChunkCells));
                    }
                }
            }
            foreach (var key in dirty)
                _chunks.Remove(key);
            InvalidateVisual();
        }

        private double Scale => Cell * _zoom;
        private double ExtentW => (_document?.Width ?? 0) * Scale;
        private double ExtentH => (_document?.Height ?? 0) * Scale;

        /// <summary>
        /// Screen position of cell (0, 0).
        /// </summary>
        private Point Origin => new Point(
            ExtentW < _viewport.Width ? Math.Floor((_viewport.Width - ExtentW) / 2) : -_offset.X,
            ExtentH < _viewport.Height ? Math.Floor((_viewport.Height - ExtentH) / 2) : -_offset.Y);

        private (int X, int Y) CellAt(Point point)
        {
            var (x, y) = MapPoint(point);
            return ((int)Math.Floor(x), (int)Math.Floor(y));
        }

        /// <summary>
        /// Map position in cells, with the fraction inside the cell.
        /// </summary>
        private (double X, double Y) MapPoint(Point point)
        {
            var origin = Origin;
            return ((point.X - origin.X) / Scale, (point.Y - origin.Y) / Scale);
        }

        private Rect CellRect(int x, int y, int w = 1, int h = 1)
        {
            var origin = Origin;
            return new Rect(origin.X + x * Scale, origin.Y + y * Scale, w * Scale, h * Scale);
        }

        /// <summary>
        /// Zooms keeping the map point under anchor in place. _zoom is set before Editor.Zoom so the property change
        /// coming back from the view model is ignored.
        /// </summary>
        private void ZoomAt(double zoom, Point anchor)
        {
            zoom = Math.Clamp(zoom, 0.25, 8);
            var origin = Origin;
            var mapX = (anchor.X - origin.X) / Scale;
            var mapY = (anchor.Y - origin.Y) / Scale;
            _zoom = zoom;
            _offset = new Vector(mapX * Scale - anchor.X, mapY * Scale - anchor.Y);
            UpdateScroll();
            InvalidateVisual();
            if (Editor != null)
                Editor.Zoom = zoom;
        }

        public void Focus(int x, int y)
        {
            _hover = (x, y);
            CenterOn(x + 0.5, y + 0.5);
        }

        /// <summary>
        /// Scrolls so the map point (x, y) in cells is at the middle of the view.
        /// </summary>
        public void CenterOn(double x, double y)
        {
            _offset = new Vector(x * Scale - _viewport.Width / 2, y * Scale - _viewport.Height / 2);
            UpdateScroll();
            InvalidateVisual();
        }

        /// <summary>
        /// Raised after scroll, zoom, resize or document changes; the minimap redraws its view rectangle.
        /// </summary>
        public event Action ViewChanged;

        /// <summary>
        /// The visible part of the map in cells (may extend past the map when it is smaller than the view).
        /// </summary>
        public Rect VisibleCells
        {
            get
            {
                var origin = Origin;
                return new Rect(-origin.X / Scale, -origin.Y / Scale, _viewport.Width / Scale, _viewport.Height / Scale);
            }
        }

        private void UpdateScroll()
        {
            _offset.X = Math.Clamp(_offset.X, 0, Math.Max(0, ExtentW - _viewport.Width));
            _offset.Y = Math.Clamp(_offset.Y, 0, Math.Max(0, ExtentH - _viewport.Height));
            ScrollOwner?.InvalidateScrollInfo();
            if (Editor != null && _document != null)
            {
                var visible = VisibleCells;
                Editor.ViewCenter = (visible.X + visible.Width / 2, visible.Y + visible.Height / 2);
            }
            ViewChanged?.Invoke();
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

        private WriteableBitmap Chunk(int cx, int cy)
        {
            if (_chunks.TryGetValue((cx, cy), out var cached))
                return cached;

            var doc = _document;
            var assets = Editor.Assets;
            var x0 = cx * ChunkCells;
            var y0 = cy * ChunkCells;
            var cellsW = Math.Min(ChunkCells, doc.Width - x0);
            var cellsH = Math.Min(ChunkCells, doc.Height - y0);
            var cell = assets?.CellPixels ?? Cell;
            var width = cellsW * cell;
            var height = cellsH * cell;
            var pixels = new uint[width * height];
            Array.Fill(pixels, 0xFF000000u);

            // Ids a 5.50 client does not have stay black: no tile drawn, no object drawn.
            var limit = Editor.Limit550;
            if (assets != null && Editor.ShowTiles)
            {
                for (int y = y0; y < y0 + cellsH; y++)
                {
                    for (int x = x0; x < x0 + cellsW; x++)
                    {
                        var tile = doc.Map.Tiles[y * doc.Width + x];
                        if (limit is (int tiles, _) && tile >= tiles)
                            continue;

                        assets.DrawTile(pixels, width, height, (x - x0) * cell, (y - y0) * cell, tile);
                    }
                }
            }
            if (assets != null && Editor.ShowObjects)
            {
                // Rows below the chunk can stack objects up into it; draw top to bottom like the client.
                var yEnd = Math.Min(doc.Height, y0 + cellsH + assets.Objects.MaxHeight);
                for (int y = y0; y < yEnd; y++)
                {
                    for (int x = Math.Max(0, x0 - 1); x < Math.Min(doc.Width, x0 + cellsW + 1); x++)
                    {
                        var objectId = doc.Map.Objects[y * doc.Width + x];
                        if (limit is (_, int objects) && objectId > objects)
                            continue;

                        assets.DrawObject(pixels, width, height, (x - x0) * cell, (y - y0) * cell, objectId);
                    }
                }
            }

            var bitmap = new WriteableBitmap(width, height, 96, 96, PixelFormats.Pbgra32, null);
            bitmap.WritePixels(new Int32Rect(0, 0, width, height), pixels, width * 4, 0);
            bitmap.Freeze();
            _chunks[(cx, cy)] = bitmap;
            return bitmap;
        }

        private (int X, int Y) DragDelta => _hover is (int hx, int hy) ? (hx - _dragStart.X, hy - _dragStart.Y) : (0, 0);

        protected override void OnRender(DrawingContext dc)
        {
            dc.DrawRectangle(BackgroundBrush, null, new Rect(0, 0, ActualWidth, ActualHeight));
            var doc = _document;
            var editor = Editor;
            if (doc == null || editor == null)
                return;

            var mapRect = CellRect(0, 0, doc.Width, doc.Height);
            dc.DrawRectangle(MapBackBrush, MapBorderPen, mapRect);

            var (vx0, vy0) = CellAt(new Point(0, 0));
            var (vx1, vy1) = CellAt(new Point(_viewport.Width, _viewport.Height));
            vx0 = Math.Max(0, vx0);
            vy0 = Math.Max(0, vy0);
            vx1 = Math.Min(doc.Width - 1, vx1);
            vy1 = Math.Min(doc.Height - 1, vy1);
            if (vx1 < vx0 || vy1 < vy0)
                return;

            var (cx0, cy0, cx1, cy1) = (vx0 / ChunkCells, vy0 / ChunkCells, vx1 / ChunkCells, vy1 / ChunkCells);
            var chunks = new DrawingGroup();
            RenderOptions.SetBitmapScalingMode(chunks, ScalingMode(editor.Assets));
            using (var context = chunks.Open())
            {
                for (int cy = cy0; cy <= cy1; cy++)
                {
                    for (int cx = cx0; cx <= cx1; cx++)
                    {
                        var chunk = Chunk(cx, cy);
                        var cellsW = Math.Min(ChunkCells, doc.Width - cx * ChunkCells);
                        var cellsH = Math.Min(ChunkCells, doc.Height - cy * ChunkCells);
                        context.DrawImage(chunk, CellRect(cx * ChunkCells, cy * ChunkCells, cellsW, cellsH));
                    }
                }
            }
            dc.DrawDrawing(chunks);

            // A 6.51 chunk is about 9 MB; keep only the visible ones once the cache grows.
            if (_chunks.Count > 24)
            {
                foreach (var key in _chunks.Keys.Where(k => k.Item1 < cx0 || k.Item1 > cx1 || k.Item2 < cy0 || k.Item2 > cy1).ToList())
                    _chunks.Remove(key);
            }

            if (editor.ShowBlocks || editor.ShowCollision || editor.Selection.Count > 0)
            {
                var blocks = new StreamGeometry();
                var collision = new StreamGeometry();
                var edges = new StreamGeometry();
                var selection = new StreamGeometry();
                var edgeWidth = Math.Max(2, Scale * 0.12);
                using (var blockContext = blocks.Open())
                using (var collisionContext = collision.Open())
                using (var edgeContext = edges.Open())
                using (var selectionContext = selection.Open())
                {
                    for (int y = vy0; y <= vy1; y++)
                    {
                        for (int x = vx0; x <= vx1; x++)
                        {
                            var isBlock = doc.Blocks.Contains(x, y);
                            if (editor.ShowBlocks && isBlock)
                            {
                                AddRect(blockContext, CellRect(x, y));
                            }
                            else if (editor.ShowCollision && isBlock == false && editor.IsBlocked(x, y))
                            {
                                AddRect(collisionContext, CellRect(x, y));
                            }
                            else if (editor.ShowCollision && isBlock == false && editor.Assets != null)
                            {
                                // Partial SObj collision blocks moving across single sides (S=1, N=2, W=4, E=8).
                                var bits = editor.Assets.Objects.Find(doc.Map.Objects[y * doc.Width + x])?.Collision ?? 0;
                                var r = CellRect(x, y);
                                r.Inflate(-edgeWidth / 2, -edgeWidth / 2);
                                if ((bits & 2) != 0)
                                    AddLine(edgeContext, r.TopLeft, r.TopRight);
                                if ((bits & 1) != 0)
                                    AddLine(edgeContext, r.BottomLeft, r.BottomRight);
                                if ((bits & 4) != 0)
                                    AddLine(edgeContext, r.TopLeft, r.BottomLeft);
                                if ((bits & 8) != 0)
                                    AddLine(edgeContext, r.TopRight, r.BottomRight);
                            }

                            if (editor.Selection.Contains((x, y)))
                                AddRect(selectionContext, CellRect(x, y));
                        }
                    }
                }
                dc.DrawGeometry(CollisionBrush, null, collision);
                dc.DrawGeometry(null, new Pen(EdgeBrush, edgeWidth), edges);
                dc.DrawGeometry(BlockBrush, null, blocks);
                dc.DrawGeometry(SelectionBrush, null, selection);
            }

            if (editor.ShowGrid && Scale >= 8)
            {
                var grid = new StreamGeometry();
                using (var context = grid.Open())
                {
                    for (int x = vx0; x <= vx1 + 1; x++)
                    {
                        var r = CellRect(x, vy0, 0, vy1 - vy0 + 1);
                        context.BeginFigure(r.TopLeft, false, false);
                        context.LineTo(r.BottomLeft, true, false);
                    }
                    for (int y = vy0; y <= vy1 + 1; y++)
                    {
                        var r = CellRect(vx0, y, vx1 - vx0 + 1, 0);
                        context.BeginFigure(r.TopLeft, false, false);
                        context.LineTo(r.TopRight, true, false);
                    }
                }
                dc.DrawGeometry(null, GridPen, grid);
            }

            if (editor.ShowDoors)
            {
                foreach (var door in doc.Doors)
                {
                    if (door.Y < vy0 || door.Y > vy1 || door.X + door.Width < vx0 || door.X > vx1)
                        continue;

                    var pen = ReferenceEquals(door, editor.SelectedMapDoor) ? SelectedPen : door.Opened ? DoorOpenPen : DoorClosePen;
                    dc.DrawRectangle(null, pen, CellRect(door.X, door.Y, door.Width, 1));
                }
            }

            if (editor.ShowMobs)
            {
                // Spawn rows with the same area share one outline and one label listing every mob.
                foreach (var group in doc.Mobs.GroupBy(m => (m.Left, m.Top, m.Right, m.Bottom)))
                {
                    var (left, top, right, bottom) = group.Key;
                    var rect = CellRect(left, top, right - left + 1, bottom - top + 1);
                    var selected = group.Any(editor.SelectedEntities.Contains);
                    dc.DrawRectangle(selected ? MobFill : null, selected ? SelectedPen : MobPen, rect);
                    if (editor.ShowSprites && editor.Assets != null)
                    {
                        var kinds = group.Where(m => m.Info != null).GroupBy(m => m.Mob).Select(g => g.First().Info).ToList();
                        var cx = (left + right) / 2.0;
                        var cy = (top + bottom) / 2.0;
                        for (int i = 0; i < kinds.Count; i++)
                            DrawSprite(dc, editor.Assets, kinds[i], 2, cx + i - (kinds.Count - 1) / 2.0, cy);
                    }
                    var label = string.Join("\n", group.Select(m => $"{(m.Name == "" ? m.Mob.ToString() : m.Name)} ×{m.Count}"));
                    DrawLabel(dc, label, new Point(Math.Max(rect.X, 0) + 2, Math.Max(rect.Y, 0) + 2), MobLabelBrush);
                }
                if (editor.SelectedMob != null && editor.SelectedEntities.Contains(editor.SelectedMob))
                {
                    var mob = editor.SelectedMob;
                    foreach (var (cx, cy) in MobCorners(mob))
                    {
                        var r = CellRect(cx, cy);
                        r.Inflate(-Scale * 0.3, -Scale * 0.3);
                        dc.DrawRectangle(Brushes.White, null, r);
                    }
                }
            }

            if (editor.ShowWarps)
            {
                foreach (var warp in doc.Warps)
                {
                    if (warp.X < vx0 || warp.X > vx1 || warp.Y < vy0 || warp.Y > vy1)
                        continue;

                    var rect = CellRect(warp.X, warp.Y);
                    rect.Inflate(-Scale * 0.15, -Scale * 0.15);
                    dc.DrawRectangle(WarpBrush, editor.SelectedEntities.Contains(warp) ? SelectedPen : null, rect);
                    if (_zoom >= 0.75)
                        DrawLabel(dc, $"→{warp.DestLabel}", new Point(rect.X, rect.Bottom + Scale * 0.15), Brushes.Cyan);
                }
            }

            if (editor.ShowNpcs)
            {
                // Sprites reach up into the rows above, so draw lower rows last like the client.
                foreach (var npc in doc.Npcs.OrderBy(n => n.Y))
                {
                    if (npc.X < vx0 || npc.X > vx1 || npc.Y < vy0 || npc.Y > vy1 + 3)
                        continue;

                    var pen = editor.SelectedEntities.Contains(npc) ? SelectedPen : null;
                    var sprite = editor.ShowSprites && editor.Assets != null ? DrawSprite(dc, editor.Assets, npc.Info, npc.DirectionIndex, npc.X, npc.Y) : Rect.Empty;
                    if (sprite.IsEmpty)
                        DrawNpc(dc, npc.X, npc.Y, npc.DirectionIndex, NpcBrush, pen);
                    else if (pen != null)
                        dc.DrawRectangle(null, pen, Rect.Union(sprite, CellRect(npc.X, npc.Y)));
                    if (_zoom >= 0.75)
                    {
                        var rect = CellRect(npc.X, npc.Y);
                        DrawLabel(dc, npc.Name == "" ? npc.Npc.ToString() : npc.Name, new Point(rect.X, rect.Bottom), Brushes.White);
                    }
                }
            }

            if (_drag == DragKind.Move)
                DrawMoveGhost(dc, editor);

            if (_hover is (int hx, int hy) && doc.Map.Contains(hx, hy) && _drag != DragKind.Move)
            {
                if (editor.Tool == EditTool.Door && editor.SelectedDoorModel != null)
                    dc.DrawRectangle(null, editor.PlaceDoorOpened ? DoorOpenPen : DoorClosePen, CellRect(hx, hy, Math.Max(1, editor.SelectedDoorModel.Pairs.Count), 1));
                else
                    dc.DrawRectangle(null, HoverPen, CellRect(hx, hy));
            }

            if ((_drag == DragKind.RubberBand || _drag == DragKind.RectFill || _drag == DragKind.MobArea || _drag == DragKind.ContextArea || _drag == DragKind.ResizeMob) &&
                _hover is (int ex, int ey))
            {
                var (sx, sy) = _drag == DragKind.ResizeMob ? OppositeCorner(_resizeMob, _resizeCorner) : _dragStart;
                dc.DrawRectangle(null, DragPen, CellRect(Math.Min(sx, ex), Math.Min(sy, ey), Math.Abs(ex - sx) + 1, Math.Abs(ey - sy) + 1));
            }

            // The cells a context menu acts on stay marked until the menu closes.
            if (_contextArea is Int32Rect area)
                dc.DrawRectangle(ContextAreaBrush, ContextAreaPen, CellRect(area.X, area.Y, area.Width, area.Height));
        }

        /// <summary>
        /// Idle sprite of an npc/mob row standing on cell (x, y); fractional cells center sprites inside mob areas.
        /// Returns the drawn screen rect, or Rect.Empty when the look has no sprite.
        /// </summary>
        private Rect DrawSprite(DrawingContext dc, ClientAssets assets, NameEntry info, int direction, double x, double y)
        {
            if (info == null)
                return Rect.Empty;

            var frame = assets.MonsterFrame(info.Look, info.Color, direction);
            var bitmap = Thumbnail.Monster(assets, info.Look, info.Color, direction);
            if (frame == null || bitmap == null)
                return Rect.Empty;

            // Frame offsets are relative to the feet: centered horizontally, a few pixels above the cell bottom.
            var cell = CellRect(0, 0);
            var feetX = cell.X + (x + 0.5) * Scale;
            var feetY = cell.Y + (y + FeetY / (double)Cell) * Scale;
            var unit = Scale / assets.CellPixels;
            var rect = new Rect(feetX + frame.Left * unit, feetY + frame.Top * unit, frame.Width * unit, frame.Height * unit);
            var sprite = new DrawingGroup();
            RenderOptions.SetBitmapScalingMode(sprite, ScalingMode(assets));
            using (var context = sprite.Open())
                context.DrawImage(bitmap, rect);
            dc.DrawDrawing(sprite);
            return rect;
        }

        /// <summary>
        /// Shrinking 6.51 double-resolution bitmaps with nearest neighbor drops every other pixel; filter until they
        /// are shown at least 1:1.
        /// </summary>
        private BitmapScalingMode ScalingMode(ClientAssets assets)
        {
            return assets != null && Scale < assets.CellPixels ? BitmapScalingMode.Linear : BitmapScalingMode.NearestNeighbor;
        }

        /// <summary>
        /// NPC marker: a circle with a triangle pointing in the spawn direction (TOP, RIGHT, BOTTOM, LEFT).
        /// </summary>
        private void DrawNpc(DrawingContext dc, int x, int y, int direction, Brush brush, Pen pen)
        {
            var rect = CellRect(x, y);
            var center = new Point(rect.X + rect.Width / 2, rect.Y + rect.Height / 2);
            var radius = Scale * 0.32;
            dc.DrawEllipse(brush, pen, center, radius, radius);

            var (ux, uy) = direction switch
            {
                0 => (0.0, -1.0),
                1 => (1.0, 0.0),
                3 => (-1.0, 0.0),
                _ => (0.0, 1.0),
            };
            var tip = new Point(center.X + ux * Scale * 0.5, center.Y + uy * Scale * 0.5);
            var baseCenter = new Point(center.X + ux * radius * 0.2, center.Y + uy * radius * 0.2);
            var side = new Vector(-uy * radius * 0.75, ux * radius * 0.75);
            var arrow = new StreamGeometry();
            using (var context = arrow.Open())
            {
                context.BeginFigure(tip, true, true);
                context.PolyLineTo(new[] { baseCenter + side, baseCenter - side }, true, false);
            }
            dc.DrawGeometry(Brushes.White, null, arrow);
        }

        private void DrawMoveGhost(DrawingContext dc, MainWindowViewModel editor)
        {
            var (dx, dy) = DragDelta;
            if (dx == 0 && dy == 0)
                return;

            var ghost = new StreamGeometry();
            using (var context = ghost.Open())
            {
                foreach (var (x, y) in editor.Selection)
                {
                    if (editor.FloatingPaste || editor.MoveTiles || editor.MoveBlocks || _document.Get(x, y).Object != 0)
                        AddRect(context, CellRect(x + dx, y + dy));
                }
            }
            dc.DrawGeometry(GhostBrush, GhostPen, ghost);

            foreach (var entity in editor.SelectedEntities)
            {
                if (entity is NpcSpawn npc)
                {
                    DrawNpc(dc, npc.X + dx, npc.Y + dy, npc.DirectionIndex, GhostBrush, GhostPen);
                }
                else if (entity is WarpEntry warp)
                {
                    var rect = CellRect(warp.X + dx, warp.Y + dy);
                    rect.Inflate(-Scale * 0.15, -Scale * 0.15);
                    dc.DrawRectangle(GhostBrush, GhostPen, rect);
                }
                else if (entity is MobSpawn mob)
                {
                    dc.DrawRectangle(null, GhostPen, CellRect(mob.Left + dx, mob.Top + dy, mob.Right - mob.Left + 1, mob.Bottom - mob.Top + 1));
                }
            }
        }

        private static IEnumerable<(int X, int Y)> MobCorners(MobSpawn mob)
        {
            yield return (mob.Left, mob.Top);
            yield return (mob.Right, mob.Top);
            yield return (mob.Left, mob.Bottom);
            yield return (mob.Right, mob.Bottom);
        }

        private static (int X, int Y) OppositeCorner(MobSpawn mob, (int X, int Y) corner)
        {
            return (corner.X == mob.Left ? mob.Right : mob.Left, corner.Y == mob.Top ? mob.Bottom : mob.Top);
        }

        private static void AddLine(StreamGeometryContext context, Point from, Point to)
        {
            context.BeginFigure(from, false, false);
            context.LineTo(to, true, false);
        }

        private static void AddRect(StreamGeometryContext context, Rect rect)
        {
            context.BeginFigure(rect.TopLeft, true, true);
            context.PolyLineTo(new[] { rect.TopRight, rect.BottomRight, rect.BottomLeft }, false, false);
        }

        private void DrawLabel(DrawingContext dc, string text, Point at, Brush brush)
        {
            var formatted = new FormattedText(text, CultureInfo.CurrentUICulture, FlowDirection.LeftToRight, LabelFace, 11, brush,
                                              VisualTreeHelper.GetDpi(this).PixelsPerDip);
            dc.DrawRectangle(LabelBackBrush, null, new Rect(at, new Size(formatted.Width + 4, formatted.Height)));
            dc.DrawText(formatted, new Point(at.X + 2, at.Y));
        }

        private SelectMode ModifierMode()
        {
            var modifiers = Keyboard.Modifiers & (ModifierKeys.Control | ModifierKeys.Shift);
            if (modifiers == (ModifierKeys.Control | ModifierKeys.Shift))
                return SelectMode.Remove;
            else if (modifiers != ModifierKeys.None)
                return SelectMode.Add;
            else
                return SelectMode.Replace;
        }

        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            base.OnMouseDown(e);
            Focus();
            var editor = Editor;
            if (editor == null || _document == null)
                return;

            if (e.ChangedButton == MouseButton.Middle || (e.ChangedButton == MouseButton.Left && _spaceDown))
            {
                _panStart = e.GetPosition(this);
                _panOffset = _offset;
                CaptureMouse();
                return;
            }

            var (x, y) = CellAt(e.GetPosition(this));
            _dragStart = (x, y);
            if (e.ChangedButton == MouseButton.Right)
            {
                _drag = DragKind.ContextArea;
                CaptureMouse();
                return;
            }
            if (e.ChangedButton != MouseButton.Left)
                return;

            CaptureMouse();
            if (editor.Tool == EditTool.Select)
            {
                if (e.ClickCount == 2)
                    editor.SelectSame(x, y, ModifierMode());
                else
                    BeginSelectDrag(editor, x, y, MapPoint(e.GetPosition(this)));
            }
            else if (editor.Tool == EditTool.Brush || editor.Tool == EditTool.Eraser)
            {
                editor.Paint(new[] { (x, y) }, editor.Tool == EditTool.Eraser, merge: false);
                _stroking = true;
                _lastStroke = (x, y);
            }
            else if (editor.Tool == EditTool.Rect)
            {
                _drag = DragKind.RectFill;
            }
            else if (editor.Tool == EditTool.Mob)
            {
                _drag = DragKind.MobArea;
            }
            else if (editor.Tool == EditTool.Fill)
            {
                editor.FloodFill(x, y);
            }
            else if (editor.Tool == EditTool.Eyedropper)
            {
                editor.Pick(x, y);
            }
            else if (editor.Tool == EditTool.Door)
            {
                var door = editor.DoorAt(x, y);
                if (door != null && editor.SelectedDoorModel == null)
                    editor.SelectedMapDoor = door;
                else
                    editor.PlaceDoor(x, y);
            }
            InvalidateVisual();
        }

        /// <summary>
        /// Select tool press. Without modifiers, dragging something already selected moves it and any other drag is
        /// a rubber band; a click without dragging selects what is under the cursor (see OnMouseUp).
        /// Alt grabs what is under the cursor and moves it at once, tall objects by their picture.
        /// Shift/Ctrl start a rubber band that adds, Ctrl+Shift one that removes. A Ctrl click without dragging
        /// toggles the thing under the cursor instead.
        /// </summary>
        private void BeginSelectDrag(MainWindowViewModel editor, int x, int y, (double X, double Y) point)
        {
            var modifiers = Keyboard.Modifiers;
            _altClicked = modifiers == ModifierKeys.Alt;
            _dragMode = modifiers == ModifierKeys.Alt ? SelectMode.Replace : ModifierMode();
            _toggleClick = modifiers == ModifierKeys.Control;
            if (_dragMode != SelectMode.Replace)
            {
                _drag = DragKind.RubberBand;
                return;
            }

            var mob = editor.SelectedMob;
            if (editor.ShowMobs && mob != null && editor.SelectedEntities.Contains(mob) && MobCorners(mob).Contains((x, y)))
            {
                _resizeMob = mob;
                _resizeCorner = (x, y);
                _drag = DragKind.ResizeMob;
                return;
            }

            Entity entity = (editor.ShowNpcs ? editor.NpcAt(x, y) : null) ?? (Entity)(editor.ShowWarps ? editor.WarpAt(x, y) : null);
            entity ??= editor.ShowMobs ? MobAt(editor, x, y) : null;
            if (modifiers == ModifierKeys.Alt)
            {
                var objectCell = ClickedObjectCell(editor, x, y, point);
                if (entity != null)
                {
                    if (editor.SelectedEntities.Contains(entity) == false)
                        editor.SelectEntity(entity, add: false);
                    _drag = DragKind.Move;
                }
                else if (objectCell is (int cx, int cy))
                {
                    if (editor.Selection.Contains((cx, cy)) == false)
                    {
                        editor.ClearSelection();
                        editor.ChangeSelection(new[] { (cx, cy) }, SelectMode.Replace);
                    }
                    _drag = DragKind.Move;
                }
                else
                {
                    _drag = DragKind.RubberBand;
                }
            }
            else
            {
                var grabbed = (entity != null && editor.SelectedEntities.Contains(entity)) || editor.Selection.Contains((x, y));
                _drag = grabbed ? DragKind.Move : DragKind.RubberBand;
            }
        }

        /// <summary>
        /// The object cell a click picks: the clicked cell when it holds an object. With Alt, the base cell of the
        /// object drawn under the cursor, so a tall object can be grabbed by its picture.
        /// </summary>
        private static (int X, int Y)? ClickedObjectCell(MainWindowViewModel editor, int x, int y, (double X, double Y) point)
        {
            if (editor.ShowObjects == false)
                return null;

            if (Keyboard.Modifiers.HasFlag(ModifierKeys.Alt))
                return editor.ObjectCellAt(point.X, point.Y);
            else
                return editor.Document.Map.Contains(x, y) && editor.Document.Get(x, y).Object != 0 ? (x, y) : null;
        }

        /// <summary>
        /// Mob areas are grabbed by their border, or anywhere inside when already selected. The smallest area wins.
        /// </summary>
        private static MobSpawn MobAt(MainWindowViewModel editor, int x, int y)
        {
            return editor.Document.Mobs
                         .Where(m => m.Contains(x, y) && (editor.SelectedEntities.Contains(m) || x == m.Left || x == m.Right || y == m.Top || y == m.Bottom))
                         .OrderBy(m => (m.Right - m.Left + 1) * (m.Bottom - m.Top + 1))
                         .FirstOrDefault();
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            var editor = Editor;
            if (editor == null || _document == null)
                return;

            var position = e.GetPosition(this);
            if (_panStart is Point start)
            {
                _offset = _panOffset - (position - start);
                UpdateScroll();
                InvalidateVisual();
                return;
            }

            var cell = CellAt(position);
            if (_hover != cell)
            {
                _hover = cell;
                editor.Hover(cell.X, cell.Y);
                InvalidateVisual();
            }

            if (_stroking && cell != _lastStroke)
            {
                editor.Paint(Line(_lastStroke, cell), editor.Tool == EditTool.Eraser, merge: true);
                _lastStroke = cell;
            }
        }

        private static IEnumerable<(int X, int Y)> Line((int X, int Y) from, (int X, int Y) to)
        {
            var steps = Math.Max(Math.Abs(to.X - from.X), Math.Abs(to.Y - from.Y));
            for (int i = 1; i <= steps; i++)
                yield return (from.X + (to.X - from.X) * i / steps, from.Y + (to.Y - from.Y) * i / steps);
        }

        protected override void OnMouseUp(MouseButtonEventArgs e)
        {
            base.OnMouseUp(e);
            ReleaseMouseCapture();
            _panStart = null;
            _stroking = false;

            var editor = Editor;
            var drag = _drag;
            _drag = DragKind.None;
            if (editor == null || _document == null || drag == DragKind.None)
                return;

            var (x, y) = CellAt(e.GetPosition(this));
            var (sx, sy) = _dragStart;
            if (drag == DragKind.RubberBand && _toggleClick && (sx, sy) == (x, y))
            {
                Entity entity = (editor.ShowNpcs ? editor.NpcAt(x, y) : null) ?? (Entity)(editor.ShowWarps ? editor.WarpAt(x, y) : null);
                entity ??= editor.ShowMobs ? MobAt(editor, x, y) : null;
                var objectCell = ClickedObjectCell(editor, x, y, MapPoint(e.GetPosition(this)));
                if (entity != null)
                    editor.ToggleEntity(entity);
                else
                    editor.ToggleCell(objectCell ?? (x, y));
            }
            else if (drag == DragKind.RubberBand && _dragMode == SelectMode.Replace && (sx, sy) == (x, y))
            {
                Entity entity = (editor.ShowNpcs ? editor.NpcAt(x, y) : null) ?? (Entity)(editor.ShowWarps ? editor.WarpAt(x, y) : null);
                entity ??= editor.ShowMobs ? MobAt(editor, x, y) : null;
                if (entity != null)
                    editor.SelectEntity(entity, add: false);
                else
                    editor.SelectRect(sx, sy, x, y, _dragMode);
            }
            else if (drag == DragKind.RubberBand)
            {
                editor.SelectRect(sx, sy, x, y, _dragMode);
            }
            else if (drag == DragKind.Move)
            {
                editor.MoveSelection(x - sx, y - sy);
            }
            else if (drag == DragKind.ResizeMob)
            {
                var (fx, fy) = OppositeCorner(_resizeMob, _resizeCorner);
                editor.SetMobArea(_resizeMob, fx, fy, x, y);
            }
            else if (drag == DragKind.RectFill)
            {
                editor.FillRect(sx, sy, x, y);
            }
            else if (drag == DragKind.MobArea)
            {
                if (editor.SelectedMob != null)
                    editor.SetMobArea(editor.SelectedMob, sx, sy, x, y);
                else
                    editor.AddMob(sx, sy, x, y);
            }
            else if (drag == DragKind.ContextArea)
            {
                var menu = (sx, sy) == (x, y) ? PointMenu(editor, x, y) : AreaMenu(editor, sx, sy, x, y);
                _contextArea = new Int32Rect(Math.Min(sx, x), Math.Min(sy, y), Math.Abs(x - sx) + 1, Math.Abs(y - sy) + 1);
                menu.Closed += (_, _) =>
                {
                    _contextArea = null;
                    InvalidateVisual();
                };
                menu.PlacementTarget = this;
                menu.Placement = PlacementMode.MousePoint;
                menu.IsOpen = true;
            }
            InvalidateVisual();
        }

        protected override void OnMouseLeave(MouseEventArgs e)
        {
            base.OnMouseLeave(e);
            _hover = null;
            Editor?.Hover(-1, -1);
            InvalidateVisual();
        }

        protected override void OnMouseWheel(MouseWheelEventArgs e)
        {
            base.OnMouseWheel(e);
            if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control))
                ZoomAt(e.Delta > 0 ? _zoom * 1.25 : _zoom / 1.25, e.GetPosition(this));
            else if (Keyboard.Modifiers.HasFlag(ModifierKeys.Shift))
                SetHorizontalOffset(_offset.X - e.Delta);
            else
                SetVerticalOffset(_offset.Y - e.Delta);
            e.Handled = true;
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            base.OnKeyDown(e);
            var editor = Editor;
            if (e.Key == Key.Space)
            {
                _spaceDown = true;
                Cursor = Cursors.SizeAll;
                e.Handled = true;
            }
            else if (editor != null && Keyboard.Modifiers == ModifierKeys.None && (e.Key == Key.Left || e.Key == Key.Right || e.Key == Key.Up || e.Key == Key.Down))
            {
                // Arrow keys nudge the selection by one cell.
                var dx = e.Key == Key.Left ? -1 : e.Key == Key.Right ? 1 : 0;
                var dy = e.Key == Key.Up ? -1 : e.Key == Key.Down ? 1 : 0;
                editor.MoveSelection(dx, dy);
                e.Handled = true;
            }
        }

        protected override void OnKeyUp(KeyEventArgs e)
        {
            base.OnKeyUp(e);
            if (e.Key == Key.Space)
            {
                _spaceDown = false;
                Cursor = null;
                e.Handled = true;
            }
            else if (_altClicked && e.Key == Key.System && (e.SystemKey == Key.LeftAlt || e.SystemKey == Key.RightAlt))
            {
                // Releasing Alt after an Alt+click would otherwise move focus to the menu bar.
                _altClicked = false;
                e.Handled = true;
            }
        }

        private static MenuItem Item(string header, Action action, bool enabled = true)
        {
            var item = new MenuItem { Header = header, IsEnabled = enabled };
            item.Click += (s, e) => action();
            return item;
        }

        private static MenuItem Title(string header)
        {
            return new MenuItem { Header = header, IsEnabled = false, FontWeight = FontWeights.Bold };
        }

        /// <summary>
        /// Right click on one cell.
        /// </summary>
        private ContextMenu PointMenu(MainWindowViewModel editor, int x, int y)
        {
            var menu = new ContextMenu();
            var doc = _document;
            if (doc.Map.Contains(x, y) == false)
            {
                menu.Items.Add(Title($"({x}, {y}) 맵 밖"));
                return menu;
            }

            var cell = doc.Get(x, y);
            menu.Items.Add(Title($"({x}, {y})  타일 {cell.Tile} / 오브젝트 {cell.Object}"));
            menu.Items.Add(new Separator());

            var npc = editor.NpcAt(x, y);
            if (npc != null)
            {
                var npcMenu = new MenuItem { Header = $"NPC: {npc.Npc} {npc.Name}" };
                npcMenu.Items.Add(Item("선택", () => editor.SelectEntity(npc, add: false)));
                foreach (var direction in NpcSpawn.Directions)
                {
                    var item = Item($"방향 {direction}", () => npc.Direction = direction);
                    item.IsChecked = npc.DirectionIndex == Array.IndexOf(NpcSpawn.Directions, direction);
                    npcMenu.Items.Add(item);
                }
                npcMenu.Items.Add(Item("NPC 바꾸기...", () =>
                {
                    var entry = PickerWindow.Show("NPC 선택", editor.NpcChoices, editor.Assets);
                    if (entry != null)
                        npc.Npc = entry.Id;
                }));
                npcMenu.Items.Add(Item("삭제", () => editor.RemoveEntity(npc)));
                menu.Items.Add(npcMenu);
            }

            var warp = editor.WarpAt(x, y);
            if (warp != null)
            {
                var warpMenu = new MenuItem { Header = $"워프 → {warp.Dest} {warp.DestName}" };
                warpMenu.Items.Add(Item("선택", () => editor.SelectEntity(warp, add: false)));
                warpMenu.Items.Add(Item("목적지 맵 열기", () => _ = editor.OpenWarpDestination(warp), warp.DestMap != null));
                warpMenu.Items.Add(Item("삭제", () => editor.RemoveEntity(warp)));
                menu.Items.Add(warpMenu);
            }

            var door = editor.DoorAt(x, y);
            if (door != null)
            {
                var doorMenu = new MenuItem { Header = $"문 {door.Model.Id} ({(door.Opened ? "열림" : "닫힘")})" };
                doorMenu.Items.Add(Item(door.Opened ? "닫기" : "열기", () => editor.ToggleDoor(door)));
                doorMenu.Items.Add(Item("선택", () => editor.SelectedMapDoor = door));
                doorMenu.Items.Add(Item("삭제 (오브젝트 지우기)", () => editor.DeleteDoor(door)));
                menu.Items.Add(doorMenu);
            }

            var mobs = doc.Mobs.Where(m => m.Contains(x, y)).ToList();
            if (mobs.Count > 0)
            {
                var mobMenu = new MenuItem { Header = $"이 칸의 몹 스폰 ({mobs.Count})" };
                foreach (var mob in mobs)
                    mobMenu.Items.Add(Item($"{mob.Mob} {mob.Name} ×{mob.Count} ({mob.Left}, {mob.Top})-({mob.Right}, {mob.Bottom})", () => editor.SelectEntity(mob, add: false)));
                menu.Items.Add(mobMenu);
            }
            if (npc != null || warp != null || door != null || mobs.Count > 0)
                menu.Items.Add(new Separator());

            menu.Items.Add(Item("NPC 추가...", () => editor.AddNpc(x, y)));
            menu.Items.Add(Item("워프 추가...", () => editor.AddWarp(x, y)));
            menu.Items.Add(Item("몹 스폰 추가 (이 칸)...", () => editor.AddMob(x, y, x, y)));
            menu.Items.Add(new Separator());

            menu.Items.Add(Item(cell.Block ? "블록 해제" : "블록 설정", () => editor.SetBlock(new[] { (x, y) }, cell.Block == false)));
            menu.Items.Add(Item($"타일 스포이트 ({cell.Tile})", () => editor.PickTile(x, y)));
            menu.Items.Add(Item($"오브젝트 스포이트 ({cell.Object})", () => editor.PickObject(x, y), cell.Object != 0));
            menu.Items.Add(Item($"브러시 타일 {editor.SelectedTile} 칠하기", () => editor.EditCells(new[] { (x, y) }, c => { c.Tile = (ushort)editor.SelectedTile; return c; })));
            menu.Items.Add(Item($"브러시 오브젝트 {editor.SelectedObject} 놓기", () => editor.EditCells(new[] { (x, y) }, c => { c.Object = (ushort)editor.SelectedObject; return c; })));
            menu.Items.Add(Item("오브젝트 지우기", () => editor.EditCells(new[] { (x, y) }, c => { c.Object = 0; return c; }), cell.Object != 0));
            menu.Items.Add(Item("붙여넣기 (여기)", () => editor.PasteAt(x, y), editor.Clipboard != null));
            menu.Items.Add(new Separator());

            menu.Items.Add(Item("좌표 복사", () => editor.CopyText($"{x}, {y}")));
            menu.Items.Add(Item($"map({doc.Id}, {x}, {y}) 복사", () => editor.CopyText($"map({doc.Id}, {x}, {y})")));
            menu.Items.Add(Item("여기를 화면 중앙으로", () => Focus(x, y)));
            return menu;
        }

        /// <summary>
        /// Right drag over an area.
        /// </summary>
        private ContextMenu AreaMenu(MainWindowViewModel editor, int x0, int y0, int x1, int y1)
        {
            var doc = _document;
            var left = Math.Clamp(Math.Min(x0, x1), 0, doc.Width - 1);
            var top = Math.Clamp(Math.Min(y0, y1), 0, doc.Height - 1);
            var right = Math.Clamp(Math.Max(x0, x1), 0, doc.Width - 1);
            var bottom = Math.Clamp(Math.Max(y0, y1), 0, doc.Height - 1);
            var cells = MainWindowViewModel.Rect(left, top, right, bottom).ToList();
            var npcs = doc.Npcs.Where(n => n.X >= left && n.X <= right && n.Y >= top && n.Y <= bottom).ToList();
            var warps = doc.Warps.Where(w => w.X >= left && w.X <= right && w.Y >= top && w.Y <= bottom).ToList();

            var menu = new ContextMenu();
            menu.Items.Add(Title($"({left}, {top})-({right}, {bottom})  {right - left + 1}×{bottom - top + 1}"));
            menu.Items.Add(new Separator());
            menu.Items.Add(Item("영역 선택", () => editor.SelectRect(left, top, right, bottom, SelectMode.Replace)));
            menu.Items.Add(Item("몹 스폰 추가...", () => editor.AddMob(left, top, right, bottom)));
            menu.Items.Add(Item("선택한 몹 스폰을 이 영역으로", () => editor.SetMobArea(editor.SelectedMob, left, top, right, bottom), editor.SelectedMob != null));
            menu.Items.Add(new Separator());

            menu.Items.Add(Item("블록 설정 (전체)", () => editor.SetBlock(cells, true)));
            menu.Items.Add(Item("블록 해제 (전체)", () => editor.SetBlock(cells, false)));
            menu.Items.Add(Item("막힌 칸만 블록 설정", () => editor.SetBlock(cells.Where(c => editor.IsBlocked(c.X, c.Y)), true)));
            menu.Items.Add(new Separator());

            menu.Items.Add(Item($"브러시 타일 {editor.SelectedTile}로 채우기", () => editor.EditCells(cells, c => { c.Tile = (ushort)editor.SelectedTile; return c; })));
            menu.Items.Add(Item($"브러시 오브젝트 {editor.SelectedObject}로 채우기", () => editor.EditCells(cells, c => { c.Object = (ushort)editor.SelectedObject; return c; })));
            menu.Items.Add(Item("오브젝트 지우기", () => editor.EditCells(cells, c => { c.Object = 0; return c; })));
            menu.Items.Add(Item("타일 지우기", () => editor.EditCells(cells, c => { c.Tile = 0; return c; })));
            menu.Items.Add(Item("복사", () =>
            {
                editor.SelectRect(left, top, right, bottom, SelectMode.Replace);
                editor.Copy();
            }));
            menu.Items.Add(Item("문 정의 만들기 (한 행)", () =>
            {
                editor.ChangeSelection(cells, SelectMode.Replace);
                editor.CreateDoorFromSelection();
            }, top == bottom));
            menu.Items.Add(new Separator());

            menu.Items.Add(Item($"영역 안 NPC 삭제 ({npcs.Count})", () =>
            {
                using (doc.Group())
                {
                    foreach (var npc in npcs)
                        editor.RemoveEntity(npc);
                }
            }, npcs.Count > 0));
            menu.Items.Add(Item($"영역 안 워프 삭제 ({warps.Count})", () =>
            {
                using (doc.Group())
                {
                    foreach (var warp in warps)
                        editor.RemoveEntity(warp);
                }
            }, warps.Count > 0));
            menu.Items.Add(Item("좌표 복사", () => editor.CopyText($"{left}, {top} - {right}, {bottom}")));
            return menu;
        }

        public bool CanHorizontallyScroll { get; set; }
        public bool CanVerticallyScroll { get; set; }
        public double ExtentWidth => Math.Max(ExtentW, _viewport.Width);
        public double ExtentHeight => Math.Max(ExtentH, _viewport.Height);
        public double ViewportWidth => _viewport.Width;
        public double ViewportHeight => _viewport.Height;
        public double HorizontalOffset => ExtentW < _viewport.Width ? 0 : _offset.X;
        public double VerticalOffset => ExtentH < _viewport.Height ? 0 : _offset.Y;
        public ScrollViewer ScrollOwner { get; set; }

        public void LineUp() => SetVerticalOffset(_offset.Y - Scale);
        public void LineDown() => SetVerticalOffset(_offset.Y + Scale);
        public void LineLeft() => SetHorizontalOffset(_offset.X - Scale);
        public void LineRight() => SetHorizontalOffset(_offset.X + Scale);
        public void PageUp() => SetVerticalOffset(_offset.Y - _viewport.Height);
        public void PageDown() => SetVerticalOffset(_offset.Y + _viewport.Height);
        public void PageLeft() => SetHorizontalOffset(_offset.X - _viewport.Width);
        public void PageRight() => SetHorizontalOffset(_offset.X + _viewport.Width);
        public void MouseWheelUp() => LineUp();
        public void MouseWheelDown() => LineDown();
        public void MouseWheelLeft() => LineLeft();
        public void MouseWheelRight() => LineRight();

        public void SetHorizontalOffset(double offset)
        {
            _offset.X = offset;
            UpdateScroll();
            InvalidateVisual();
        }

        public void SetVerticalOffset(double offset)
        {
            _offset.Y = offset;
            UpdateScroll();
            InvalidateVisual();
        }

        public Rect MakeVisible(Visual visual, Rect rectangle)
        {
            return rectangle;
        }
    }
}
