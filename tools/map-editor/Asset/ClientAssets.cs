using System.Collections.Concurrent;
using System.IO;
using MapEditor.Format;
using MapEditor.Settings;

namespace MapEditor.Asset
{
    public class FrameImage
    {
        public int Left;
        public int Top;
        public int Width;
        public int Height;
        public uint[] Pixels;
    }

    /// <summary>
    /// Tile and object graphics of one client version. 5.50 and 6.51 differ only in how they are read.
    /// </summary>
    public class ClientAssets
    {
        /// <summary>
        /// One map cell in editor units (screen pixels at 100% zoom).
        /// </summary>
        public const int CellSize = 24;

        private Epf _tileEpf;
        private Epf _objectEpf;
        private List<uint[]> _tilePalettes;
        private List<uint[]> _objectPalettes;
        private ushort[] _tileTable;
        private ushort[] _objectTable;
        private readonly ConcurrentDictionary<int, FrameImage> _tileFrames = new ConcurrentDictionary<int, FrameImage>();
        private readonly ConcurrentDictionary<int, FrameImage> _objectFrames = new ConcurrentDictionary<int, FrameImage>();
        private Epf _monsterEpf;
        private List<MonsterRecord> _monsters;
        private List<uint[]> _monsterPalettes;
        private readonly ConcurrentDictionary<(int, int, int), FrameImage> _monsterFrames = new ConcurrentDictionary<(int, int, int), FrameImage>();

        public ClientVersion Version { get; private set; }

        /// <summary>
        /// One map cell in resource pixels: 24 in 5.50, 48 in 6.51 (every tile, object and monster frame is drawn at
        /// double resolution). Frame offsets and DrawTile/DrawObject coordinates use this unit.
        /// </summary>
        public int CellPixels { get; private set; }

        /// <summary>
        /// Bitmap DPI that shows resource pixels at CellSize per cell in WPF units.
        /// </summary>
        public double Dpi => 96.0 * CellPixels / CellSize;

        public SObjTable Objects { get; private set; }
        public int TileCount => _tileEpf.Count;

        public static ClientAssets Load(ClientVersion version, string directory)
        {
            return version == ClientVersion.v550 ? Load550(directory) : Load651(directory);
        }

        /// <summary>
        /// 5.50: everything is in TILE.DAT.
        /// </summary>
        public static ClientAssets Load550(string directory)
        {
            var dat = DatArchive.Read(Path.Combine(directory, "TILE.DAT"));
            var assets = new ClientAssets { Version = ClientVersion.v550, CellPixels = 24 };
            assets._tileEpf = new Epf();
            assets._tileEpf.Append(dat.Get("tile.epf"));
            assets._objectEpf = new Epf();
            assets._objectEpf.Append(dat.Get("tilec.epf"));
            assets._tilePalettes = Pal.Read(dat.Get("tile.pal"));
            assets._objectPalettes = Pal.Read(dat.Get("tilec.pal"));
            assets._tileTable = Tbl.Read(dat.Get("tile.tbl"), shortCount: false);
            assets._objectTable = Tbl.Read(dat.Get("tilec.tbl"), shortCount: false);
            assets.Objects = SObjTable.Read(dat.Get("sobj.tbl"));

            // NPC and mob sprites; without MON.DAT the editor draws markers only.
            var monPath = Path.Combine(directory, "MON.DAT");
            if (File.Exists(monPath))
            {
                var mon = DatArchive.Read(monPath);
                assets._monsterEpf = new Epf();
                assets._monsterEpf.Append(mon.Get("monster.epf"));
                assets._monsters = MonsterDna.Read(mon.Get("monster.dna"));
                assets._monsterPalettes = Pal.Read(mon.Get("monster.pal"));
            }
            return assets;
        }

        /// <summary>
        /// 6.51 new UI mode: TILE.DAT holds SObj/PAL/TBL, images are split into TILE{n}.DAT and TILEC{n}.DAT
        /// whose EPF frame tables are concatenated from n = 0.
        /// </summary>
        public static ClientAssets Load651(string directory)
        {
            var dat = DatArchive.Read(Path.Combine(directory, "TILE.DAT"));
            var assets = new ClientAssets { Version = ClientVersion.v651, CellPixels = 48 };
            assets._tileEpf = new Epf();
            assets._objectEpf = new Epf();
            for (int n = 0; n < 100; n++)
            {
                var path = Path.Combine(directory, $"TILE{n}.DAT");
                if (File.Exists(path) == false)
                    break;

                assets._tileEpf.Append(DatArchive.Read(path).Get($"tile{n}.epf"));
            }
            for (int n = 0; n < 100; n++)
            {
                var path = Path.Combine(directory, $"TILEC{n}.DAT");
                if (File.Exists(path) == false)
                    break;

                assets._objectEpf.Append(DatArchive.Read(path).Get($"tilec{n}.epf"));
            }
            assets._tilePalettes = Pal.Read(dat.Get("tile.pal"));
            assets._objectPalettes = Pal.Read(dat.Get("tilec.pal"));
            assets._tileTable = Tbl.Read(dat.Get("tile.tbl"), shortCount: true);
            assets._objectTable = Tbl.Read(dat.Get("tilec.tbl"), shortCount: true);
            assets.Objects = SObjTable.Read(dat.Get("sobj.tbl"));

            // NPC and mob sprites: DNA/PAL in MON.DAT, images split into MON{n}.DAT like the tiles.
            var monPath = Path.Combine(directory, "MON.DAT");
            if (File.Exists(monPath))
            {
                var mon = DatArchive.Read(monPath);
                assets._monsterEpf = new Epf();
                for (int n = 0; n < 100; n++)
                {
                    var path = Path.Combine(directory, $"MON{n}.DAT");
                    if (File.Exists(path) == false)
                        break;

                    assets._monsterEpf.Append(DatArchive.Read(path).Get($"mon{n}.epf"));
                }
                assets._monsters = MonsterDna.Read(mon.Get("monster.dna"));
                assets._monsterPalettes = Pal.Read(mon.Get("monster.pal"));
            }
            return assets;
        }

        public FrameImage TileFrame(int frame)
        {
            return _tileFrames.GetOrAdd(frame, f => Decode(_tileEpf, _tilePalettes, _tileTable, f));
        }

        public FrameImage ObjectFrame(int frame)
        {
            return _objectFrames.GetOrAdd(frame, f => Decode(_objectEpf, _objectPalettes, _objectTable, f));
        }

        public bool HasMonsters => _monsters != null;

        /// <summary>
        /// Idle pose of an npc/mob xlsx look facing direction (TOP 0, RIGHT 1, BOTTOM 2, LEFT 3).
        /// Left/Top are offsets from the feet point. Null when the look has no sprite.
        /// </summary>
        public FrameImage MonsterFrame(int look, int color, int direction)
        {
            if (_monsters == null)
                return null;

            return _monsterFrames.GetOrAdd((look, color, direction), key =>
            {
                // Server sends look + 0x7FFF, the client reads record (u16)(wire - 0x8000).
                var record = (ushort)(key.Item1 + 0x7FFF - 0x8000);
                if (record >= _monsters.Count)
                    return null;

                var monster = _monsters[record];
                var anim = 1 + key.Item3;
                if (anim >= monster.FirstFrames.Length || monster.FirstFrames[anim] < 0)
                    return null;

                var index = monster.BaseFrame + monster.FirstFrames[anim];
                if (index < 0 || index >= _monsterEpf.Count || _monsterPalettes.Count == 0)
                    return null;

                var frame = _monsterEpf[index];
                if (frame.Width <= 0 || frame.Height <= 0)
                    return null;

                // color / 32 != 0 selects a fixed effect palette in the client; those are not loaded, so the
                // record palette is used and only the dye below applies.
                var palette = _monsterPalettes[monster.Palette % _monsterPalettes.Count];
                var dye = 8 * key.Item2;
                var blob = _monsterEpf.Blob(frame.Blob);
                var pixels = new uint[frame.Width * frame.Height];
                var count = Math.Min(pixels.Length, blob.Length - frame.PixelOffset);
                for (int i = 0; i < count; i++)
                {
                    int value = blob[frame.PixelOffset + i];
                    if (value >= 0x30)
                        value = (value + dye) & 0xFF;
                    pixels[i] = palette[value];
                }
                return new FrameImage { Left = frame.Left, Top = frame.Top, Width = frame.Width, Height = frame.Height, Pixels = pixels };
            });
        }

        private static FrameImage Decode(Epf epf, List<uint[]> palettes, ushort[] table, int index)
        {
            // Frame 0 is the empty frame in both tile sets.
            if (index <= 0 || index >= epf.Count)
                return null;

            var frame = epf[index];
            if (frame.Width <= 0 || frame.Height <= 0)
                return null;

            var paletteIndex = index < table.Length ? table[index] : 0;
            if (paletteIndex >= palettes.Count)
                return null;

            var palette = palettes[paletteIndex];
            var blob = epf.Blob(frame.Blob);
            var pixels = new uint[frame.Width * frame.Height];
            var count = Math.Min(pixels.Length, blob.Length - frame.PixelOffset);
            for (int i = 0; i < count; i++)
                pixels[i] = palette[blob[frame.PixelOffset + i]];

            return new FrameImage { Left = frame.Left, Top = frame.Top, Width = frame.Width, Height = frame.Height, Pixels = pixels };
        }

        /// <summary>
        /// Draws a floor tile whose cell origin is (px, py) in the target buffer.
        /// </summary>
        public void DrawTile(uint[] target, int targetWidth, int targetHeight, int px, int py, int tile)
        {
            var frame = TileFrame(tile);
            if (frame != null)
                Blit(target, targetWidth, targetHeight, px + frame.Left, py + frame.Top, frame);
        }

        /// <summary>
        /// Draws an object stack whose base cell origin is (px, py); piece k goes k cells up.
        /// </summary>
        public void DrawObject(uint[] target, int targetWidth, int targetHeight, int px, int py, int objectId)
        {
            var sobj = Objects.Find(objectId);
            if (sobj == null)
                return;

            for (int k = 0; k < sobj.Frames.Length; k++)
            {
                var frame = ObjectFrame(sobj.Frames[k]);
                if (frame != null)
                    Blit(target, targetWidth, targetHeight, px + frame.Left, py - k * CellPixels + frame.Top, frame);
            }
        }

        public static void Blit(uint[] target, int targetWidth, int targetHeight, int x, int y, FrameImage frame)
        {
            var x0 = Math.Max(0, x);
            var y0 = Math.Max(0, y);
            var x1 = Math.Min(targetWidth, x + frame.Width);
            var y1 = Math.Min(targetHeight, y + frame.Height);
            for (int ty = y0; ty < y1; ty++)
            {
                var source = (ty - y) * frame.Width - x;
                var row = ty * targetWidth;
                for (int tx = x0; tx < x1; tx++)
                {
                    var color = frame.Pixels[source + tx];
                    if (color != 0)
                        target[row + tx] = color;
                }
            }
        }
    }
}
