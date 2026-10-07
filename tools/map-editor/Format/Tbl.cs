namespace MapEditor.Format
{
    /// <summary>
    /// Frame → palette table. 5.50 has a u32 count, 6.51 a u16 count; entries are u16 with 0x7FFF = palette index.
    /// </summary>
    public static class Tbl
    {
        public static ushort[] Read(byte[] bytes, bool shortCount)
        {
            var count = shortCount ? BitConverter.ToUInt16(bytes, 0) : BitConverter.ToInt32(bytes, 0);
            var start = shortCount ? 2 : 4;
            var entries = new ushort[count];
            for (int i = 0; i < count; i++)
                entries[i] = (ushort)(BitConverter.ToUInt16(bytes, start + i * 2) & 0x7FFF);

            return entries;
        }
    }
}
