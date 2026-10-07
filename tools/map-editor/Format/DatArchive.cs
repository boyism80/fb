using System.IO;
using System.Text;

namespace MapEditor.Format
{
    /// <summary>
    /// Client DAT archive: u32 count, then count x { u32 offset, char[13] name }.
    /// The last entry is an empty end marker whose offset is the end of the last file.
    /// </summary>
    public class DatArchive
    {
        private readonly Dictionary<string, byte[]> _files = new Dictionary<string, byte[]>(StringComparer.OrdinalIgnoreCase);

        public IEnumerable<string> Names => _files.Keys;

        public static DatArchive Read(string path)
        {
            var bytes = File.ReadAllBytes(path);
            var archive = new DatArchive();
            var count = BitConverter.ToInt32(bytes, 0);
            for (int i = 0; i < count - 1; i++)
            {
                var entry = 4 + i * 17;
                var offset = BitConverter.ToInt32(bytes, entry);
                var next = BitConverter.ToInt32(bytes, entry + 17);
                var nameLength = Array.IndexOf<byte>(bytes, 0, entry + 4, 13) - (entry + 4);
                if (nameLength < 0)
                    nameLength = 13;

                var name = Encoding.ASCII.GetString(bytes, entry + 4, nameLength);
                archive._files[name] = bytes.AsSpan(offset, next - offset).ToArray();
            }
            return archive;
        }

        public bool Contains(string name)
        {
            return _files.ContainsKey(name);
        }

        public byte[] Get(string name)
        {
            if (_files.TryGetValue(name, out var bytes) == false)
                throw new FileNotFoundException($"{name} is not in the archive");

            return bytes;
        }
    }
}
