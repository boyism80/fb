using System.Buffers.Binary;
using System.IO;

namespace MapEditor.Format
{
    /// <summary>
    /// Server .map (big-endian): u16 width, u16 height, then width*height x { u16 tile, u16 object } row-major.
    /// </summary>
    public class ServerMap
    {
        public int Width { get; private set; }
        public int Height { get; private set; }
        public ushort[] Tiles { get; private set; }
        public ushort[] Objects { get; private set; }

        /// <summary>
        /// Bytes after the cell data (some shipped maps have one); written back unchanged.
        /// </summary>
        private byte[] _trailing = Array.Empty<byte>();

        public ServerMap(int width, int height)
        {
            Width = width;
            Height = height;
            Tiles = new ushort[width * height];
            Objects = new ushort[width * height];
        }

        public static ServerMap Read(string path)
        {
            var bytes = File.ReadAllBytes(path);
            var width = BinaryPrimitives.ReadUInt16BigEndian(bytes.AsSpan(0));
            var height = BinaryPrimitives.ReadUInt16BigEndian(bytes.AsSpan(2));
            var map = new ServerMap(width, height);
            for (int i = 0; i < width * height; i++)
            {
                map.Tiles[i] = BinaryPrimitives.ReadUInt16BigEndian(bytes.AsSpan(4 + i * 4));
                map.Objects[i] = BinaryPrimitives.ReadUInt16BigEndian(bytes.AsSpan(6 + i * 4));
            }
            map._trailing = bytes.AsSpan(4 + width * height * 4).ToArray();
            return map;
        }

        public byte[] ToBytes()
        {
            var bytes = new byte[4 + Width * Height * 4 + _trailing.Length];
            _trailing.CopyTo(bytes, 4 + Width * Height * 4);
            BinaryPrimitives.WriteUInt16BigEndian(bytes.AsSpan(0), (ushort)Width);
            BinaryPrimitives.WriteUInt16BigEndian(bytes.AsSpan(2), (ushort)Height);
            for (int i = 0; i < Width * Height; i++)
            {
                BinaryPrimitives.WriteUInt16BigEndian(bytes.AsSpan(4 + i * 4), Tiles[i]);
                BinaryPrimitives.WriteUInt16BigEndian(bytes.AsSpan(6 + i * 4), Objects[i]);
            }
            return bytes;
        }

        public bool Contains(int x, int y)
        {
            return x >= 0 && y >= 0 && x < Width && y < Height;
        }
    }
}
