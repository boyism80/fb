using System.IO.Compression;
using System.Text;
using Fb.Model;

namespace AdminTool.Services
{
    /// <summary>
    /// Reads collection entries from Meta.dat (same source as game server collection_mobs).
    /// </summary>
    public static class MetaDatCollection
    {
        private static readonly object Sync = new();
        private static int? _cachedTotal;
        private static string _cachedPath;

        public static int GetCollectionMobTotalCount()
        {
            try
            {
                Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);

                var path = ResolveMetaDatPath();
                if (string.IsNullOrEmpty(path) || !File.Exists(path))
                    return 0;

                lock (Sync)
                {
                    if (_cachedTotal.HasValue &&
                        string.Equals(_cachedPath, path, StringComparison.OrdinalIgnoreCase))
                        return _cachedTotal.Value;

                    var names = ReadCollectionNames(path);
                    var mobByName = BuildMobNameMap();
                    var matchedIds = new HashSet<uint>();
                    foreach (var name in names)
                    {
                        if (string.IsNullOrWhiteSpace(name))
                            continue;
                        if (mobByName.TryGetValue(name, out var mobId))
                            matchedIds.Add(mobId);
                    }

                    _cachedPath = path;
                    _cachedTotal = matchedIds.Count;
                    return _cachedTotal.Value;
                }
            }
            catch
            {
                return 0;
            }
        }

        private static string ResolveMetaDatPath()
        {
            var candidates = new[]
            {
                Path.Combine(AppContext.BaseDirectory, "Meta.dat"),
                Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "game", "Meta.dat")),
                Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "..", "resources", "meta", "Meta.dat")),
            };

            foreach (var path in candidates)
            {
                if (File.Exists(path))
                    return path;
            }

            return null;
        }

        private static Dictionary<string, uint> BuildMobNameMap()
        {
            var map = new Dictionary<string, uint>(StringComparer.Ordinal);
            foreach (var pair in Table.Mob)
            {
                if (string.IsNullOrWhiteSpace(pair.Value?.Name))
                    continue;
                map[pair.Value.Name] = pair.Key;
            }
            return map;
        }

        private static List<string> ReadCollectionNames(string path)
        {
            var bytes = File.ReadAllBytes(path);
            var offset = 0;
            var count = ReadUInt32LE(bytes, ref offset);
            var indexSize = ReadUInt32LE(bytes, ref offset);

            var entries = new List<(string Name, uint CompressedSize)>(checked((int)count));
            for (uint i = 0; i < count; i++)
            {
                var nameLen = ReadUInt16LE(bytes, ref offset);
                var chars = new char[nameLen];
                for (int w = 0; w < nameLen; w++)
                    chars[w] = (char)ReadUInt16LE(bytes, ref offset);
                var name = new string(chars);
                var compressedSize = ReadUInt32BE(bytes, ref offset);
                _ = ReadUInt32BE(bytes, ref offset); // unused
                entries.Add((name, compressedSize));
            }

            offset = 8 + (int)indexSize;
            byte[] collectionsPlain = null;
            foreach (var (name, compressedSize) in entries)
            {
                if (offset + compressedSize > bytes.Length)
                    throw new InvalidDataException($"Meta.dat blob truncated: {name}");

                var compressed = new byte[compressedSize];
                Buffer.BlockCopy(bytes, offset, compressed, 0, (int)compressedSize);
                offset += (int)compressedSize;

                if (name != "Collections")
                    continue;

                collectionsPlain = Inflate(compressed);
            }

            if (collectionsPlain == null)
                return new List<string>();

            return ParseCollectionTreeNames(collectionsPlain);
        }

        private static List<string> ParseCollectionTreeNames(byte[] data)
        {
            var offset = 0;
            var topCount = ReadUInt16BE(data, ref offset);
            var names = new List<string>(topCount);
            var cp949 = Encoding.GetEncoding(949);

            for (int i = 0; i < topCount; i++)
            {
                var name = ReadString8(data, ref offset, cp949).Trim();
                var childCount = ReadUInt16BE(data, ref offset);
                for (int c = 0; c < childCount; c++)
                    _ = ReadString16(data, ref offset, cp949);

                if (!string.IsNullOrEmpty(name))
                    names.Add(name);
            }

            return names;
        }

        private static byte[] Inflate(byte[] compressed)
        {
            // zlib (CMF/FLG header) then raw deflate fallback — matches fb::meta_dat_file
            Exception last = null;
            foreach (var skip in new[] { 2, 0 })
            {
                try
                {
                    using var input = new MemoryStream(compressed, skip, compressed.Length - skip);
                    using var deflate = new DeflateStream(input, CompressionMode.Decompress);
                    using var output = new MemoryStream();
                    deflate.CopyTo(output);
                    return output.ToArray();
                }
                catch (Exception ex)
                {
                    last = ex;
                }
            }

            throw new InvalidDataException("Failed to inflate Meta.dat entry", last);
        }

        private static string ReadString8(byte[] data, ref int offset, Encoding encoding)
        {
            if (offset >= data.Length)
                throw new InvalidDataException("Meta.dat string8 truncated");
            var len = data[offset++];
            return ReadBytesAsString(data, ref offset, len, encoding);
        }

        private static string ReadString16(byte[] data, ref int offset, Encoding encoding)
        {
            var len = ReadUInt16BE(data, ref offset);
            return ReadBytesAsString(data, ref offset, len, encoding);
        }

        private static string ReadBytesAsString(byte[] data, ref int offset, int len, Encoding encoding)
        {
            if (len < 0 || offset + len > data.Length)
                throw new InvalidDataException("Meta.dat string truncated");
            if (len == 0)
                return string.Empty;

            var text = encoding.GetString(data, offset, len);
            offset += len;
            return text;
        }

        private static ushort ReadUInt16LE(byte[] data, ref int offset)
        {
            Ensure(data, offset, 2);
            var value = (ushort)(data[offset] | (data[offset + 1] << 8));
            offset += 2;
            return value;
        }

        private static uint ReadUInt32LE(byte[] data, ref int offset)
        {
            Ensure(data, offset, 4);
            var value = (uint)(data[offset]
                               | (data[offset + 1] << 8)
                               | (data[offset + 2] << 16)
                               | (data[offset + 3] << 24));
            offset += 4;
            return value;
        }

        private static ushort ReadUInt16BE(byte[] data, ref int offset)
        {
            Ensure(data, offset, 2);
            var value = (ushort)((data[offset] << 8) | data[offset + 1]);
            offset += 2;
            return value;
        }

        private static uint ReadUInt32BE(byte[] data, ref int offset)
        {
            Ensure(data, offset, 4);
            var value = ((uint)data[offset] << 24)
                        | ((uint)data[offset + 1] << 16)
                        | ((uint)data[offset + 2] << 8)
                        | data[offset + 3];
            offset += 4;
            return value;
        }

        private static void Ensure(byte[] data, int offset, int size)
        {
            if (offset < 0 || offset + size > data.Length)
                throw new InvalidDataException("Meta.dat truncated");
        }
    }
}
