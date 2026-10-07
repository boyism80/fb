namespace MapEditor.Format
{
    public readonly struct EpfFrame
    {
        public readonly short Top;
        public readonly short Left;
        public readonly short Bottom;
        public readonly short Right;
        public readonly int Blob;
        public readonly int PixelOffset;

        public int Width => Right - Left;
        public int Height => Bottom - Top;

        public EpfFrame(short top, short left, short bottom, short right, int blob, int pixelOffset)
        {
            Top = top;
            Left = left;
            Bottom = bottom;
            Right = right;
            Blob = blob;
            PixelOffset = pixelOffset;
        }
    }

    /// <summary>
    /// EPF image set: u16 frames, u16 w, u16 h, u16 ?, u32 pixel_len, pixels, then 16-byte frame records
    /// { i16 top, left, bottom, right; u32 pixel_off; u32 mask_off } with offsets relative to the pixel data (+12).
    /// Pixels are 8-bit palette indices, uncompressed. 6.51 splits one set into chunks that are concatenated in order.
    /// </summary>
    public class Epf
    {
        private readonly List<byte[]> _blobs = new List<byte[]>();
        private readonly List<EpfFrame> _frames = new List<EpfFrame>();

        public int Count => _frames.Count;

        public EpfFrame this[int index] => _frames[index];

        public byte[] Blob(int index) => _blobs[index];

        public void Append(byte[] bytes)
        {
            var blob = _blobs.Count;
            _blobs.Add(bytes);

            var count = BitConverter.ToUInt16(bytes, 0);
            var pixelLength = BitConverter.ToInt32(bytes, 8);
            var table = 12 + pixelLength;
            for (int i = 0; i < count; i++)
            {
                var record = table + i * 16;
                _frames.Add(new EpfFrame(
                    BitConverter.ToInt16(bytes, record),
                    BitConverter.ToInt16(bytes, record + 2),
                    BitConverter.ToInt16(bytes, record + 4),
                    BitConverter.ToInt16(bytes, record + 6),
                    blob,
                    12 + BitConverter.ToInt32(bytes, record + 8)));
            }
        }
    }
}
