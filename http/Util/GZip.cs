using System.IO.Compression;

namespace http.Util
{
    public static class GZip
    {
        public static byte[] Zip(byte[] bytes)
        {
            using (var ms = new MemoryStream())
            using (var gzip = new GZipStream(ms, CompressionMode.Compress))
            {
                gzip.Write(bytes, 0, bytes.Length);
                return ms.ToArray();
            }
        }

        public static byte[] Decompress(byte[] bytes)
        {
            using var ms = new MemoryStream(bytes);
            using (var gzip = new GZipStream(ms, CompressionMode.Decompress))
            {
                return ms.ToArray();
            }
        }
    }
}
