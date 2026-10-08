using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace MapEditor.Asset
{
    /// <summary>
    /// Frozen bitmaps of single tiles and whole object stacks, cached per asset set.
    /// </summary>
    public static class Thumbnail
    {
        private static readonly ConditionalWeakTable<ClientAssets, Dictionary<int, BitmapSource>> _tiles = new ConditionalWeakTable<ClientAssets, Dictionary<int, BitmapSource>>();
        private static readonly ConditionalWeakTable<ClientAssets, Dictionary<int, BitmapSource>> _objects = new ConditionalWeakTable<ClientAssets, Dictionary<int, BitmapSource>>();

        private static readonly ConditionalWeakTable<ClientAssets, Dictionary<(int, int, int), BitmapSource>> _monsters = new ConditionalWeakTable<ClientAssets, Dictionary<(int, int, int), BitmapSource>>();

        public static BitmapSource Tile(ClientAssets assets, int id)
        {
            var cache = _tiles.GetOrCreateValue(assets);
            if (cache.TryGetValue(id, out var cached))
                return cached;

            var frame = assets.TileFrame(id);
            BitmapSource bitmap = null;
            if (frame != null)
                bitmap = Create(assets, frame.Width, frame.Height, frame.Pixels);

            cache[id] = bitmap;
            return bitmap;
        }

        public static BitmapSource Object(ClientAssets assets, int id)
        {
            var cache = _objects.GetOrCreateValue(assets);
            if (cache.TryGetValue(id, out var cached))
                return cached;

            BitmapSource bitmap = null;
            var sobj = assets.Objects.Find(id);
            if (sobj != null)
            {
                var minX = 0;
                var minY = 0;
                var cell = assets.CellPixels;
                var maxX = cell;
                var maxY = cell;
                for (int k = 0; k < sobj.Frames.Length; k++)
                {
                    var frame = assets.ObjectFrame(sobj.Frames[k]);
                    if (frame == null)
                        continue;

                    minX = Math.Min(minX, frame.Left);
                    minY = Math.Min(minY, frame.Top - k * cell);
                    maxX = Math.Max(maxX, frame.Left + frame.Width);
                    maxY = Math.Max(maxY, frame.Top - k * cell + frame.Height);
                }

                var width = maxX - minX;
                var height = maxY - minY;
                var pixels = new uint[width * height];
                assets.DrawObject(pixels, width, height, -minX, -minY, id);
                bitmap = Create(assets, width, height, pixels);
            }
            cache[id] = bitmap;
            return bitmap;
        }

        /// <summary>
        /// Objects placed side by side on one map row, drawn the way the map draws them (not cached).
        /// </summary>
        public static BitmapSource Row(ClientAssets assets, IReadOnlyList<int> ids)
        {
            var cell = assets.CellPixels;
            var minX = 0;
            var minY = 0;
            var maxX = cell * ids.Count;
            var maxY = cell;
            for (int i = 0; i < ids.Count; i++)
            {
                var sobj = assets.Objects.Find(ids[i]);
                if (sobj == null)
                    continue;

                for (int k = 0; k < sobj.Frames.Length; k++)
                {
                    var frame = assets.ObjectFrame(sobj.Frames[k]);
                    if (frame == null)
                        continue;

                    minX = Math.Min(minX, i * cell + frame.Left);
                    minY = Math.Min(minY, frame.Top - k * cell);
                    maxX = Math.Max(maxX, i * cell + frame.Left + frame.Width);
                    maxY = Math.Max(maxY, frame.Top - k * cell + frame.Height);
                }
            }

            var width = maxX - minX;
            var height = maxY - minY;
            var pixels = new uint[width * height];
            for (int i = 0; i < ids.Count; i++)
            {
                if (ids[i] != 0)
                    assets.DrawObject(pixels, width, height, i * cell - minX, -minY, ids[i]);
            }
            return Create(assets, width, height, pixels);
        }

        public static BitmapSource Monster(ClientAssets assets, int look, int color, int direction)
        {
            var cache = _monsters.GetOrCreateValue(assets);
            if (cache.TryGetValue((look, color, direction), out var cached))
                return cached;

            var frame = assets.MonsterFrame(look, color, direction);
            var bitmap = frame == null ? null : Create(assets, frame.Width, frame.Height, frame.Pixels);
            cache[(look, color, direction)] = bitmap;
            return bitmap;
        }

        /// <summary>
        /// WPF size of the bitmap is in editor units (6.51 bitmaps report half their pixel size).
        /// </summary>
        private static BitmapSource Create(ClientAssets assets, int width, int height, uint[] pixels)
        {
            var bitmap = BitmapSource.Create(width, height, assets.Dpi, assets.Dpi, PixelFormats.Pbgra32, null, pixels, width * 4);
            bitmap.Freeze();
            return bitmap;
        }
    }
}
