using System.IO;

namespace MapEditor.Format
{
    /// <summary>
    /// Server .block (little-endian): u32 count, then count x { u16 x, u16 y }.
    /// The original order (including duplicate cells some shipped files have) is kept so an unchanged file saves
    /// byte-identical; new cells are appended and removing a cell drops every copy of it.
    /// </summary>
    public class BlockFile
    {
        private readonly List<(int X, int Y)> _cells = new List<(int X, int Y)>();
        private readonly HashSet<(int X, int Y)> _lookup = new HashSet<(int X, int Y)>();

        public int Count => _lookup.Count;
        public IReadOnlyCollection<(int X, int Y)> Cells => _lookup;

        public static BlockFile Read(string path)
        {
            var file = new BlockFile();
            if (File.Exists(path) == false)
                return file;

            var bytes = File.ReadAllBytes(path);
            if (bytes.Length < 4)
                return file;

            var count = BitConverter.ToInt32(bytes, 0);
            for (int i = 0; i < count && 8 + i * 4 <= bytes.Length; i++)
            {
                var cell = ((int)BitConverter.ToUInt16(bytes, 4 + i * 4), (int)BitConverter.ToUInt16(bytes, 6 + i * 4));
                file._cells.Add(cell);
                file._lookup.Add(cell);
            }
            return file;
        }

        public byte[] ToBytes()
        {
            var bytes = new byte[4 + _cells.Count * 4];
            BitConverter.TryWriteBytes(bytes.AsSpan(0), _cells.Count);
            for (int i = 0; i < _cells.Count; i++)
            {
                BitConverter.TryWriteBytes(bytes.AsSpan(4 + i * 4), (ushort)_cells[i].X);
                BitConverter.TryWriteBytes(bytes.AsSpan(6 + i * 4), (ushort)_cells[i].Y);
            }
            return bytes;
        }

        public bool Contains(int x, int y)
        {
            return _lookup.Contains((x, y));
        }

        public void Add(int x, int y)
        {
            if (_lookup.Add((x, y)))
                _cells.Add((x, y));
        }

        public void Remove(int x, int y)
        {
            if (_lookup.Remove((x, y)))
                _cells.RemoveAll(c => c.X == x && c.Y == y);
        }

        public void Set(int x, int y, bool blocked)
        {
            if (blocked)
                Add(x, y);
            else
                Remove(x, y);
        }
    }
}
