namespace MapEditor.Format
{
    /// <summary>
    /// PAL file: u32 count, then per palette a 32-byte header (u32 anim_count at +24), anim_count x u16,
    /// and 256 x RGBX. Index 0 is transparent.
    /// </summary>
    public static class Pal
    {
        /// <summary>
        /// Returns palettes as premultiplied BGRA32 values.
        /// </summary>
        public static List<uint[]> Read(byte[] bytes)
        {
            var palettes = new List<uint[]>();
            var count = BitConverter.ToInt32(bytes, 0);
            var position = 4;
            for (int i = 0; i < count; i++)
            {
                var animations = BitConverter.ToInt32(bytes, position + 24);
                position += 32 + animations * 2;

                var colors = new uint[256];
                for (int c = 1; c < 256; c++)
                {
                    var r = bytes[position + c * 4];
                    var g = bytes[position + c * 4 + 1];
                    var b = bytes[position + c * 4 + 2];
                    colors[c] = 0xFF000000u | ((uint)r << 16) | ((uint)g << 8) | b;
                }
                position += 1024;
                palettes.Add(colors);
            }
            return palettes;
        }
    }
}
