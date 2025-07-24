using System.IO.Compression;

namespace Http.Util
{
    /// <summary>
    /// Provides utility methods for GZip compression and decompression operations.
    /// Contains static methods for compressing and decompressing byte arrays using GZip algorithm.
    /// </summary>
    public static class GZip
    {
        /// <summary>
        /// Compresses a byte array using GZip compression algorithm.
        /// </summary>
        /// <param name="bytes">The byte array to compress.</param>
        /// <returns>A compressed byte array using GZip compression.</returns>
        public static byte[] Zip(byte[] bytes)
        {
            using (var ms = new MemoryStream())
            using (var gzip = new GZipStream(ms, CompressionMode.Compress))
            {
                gzip.Write(bytes, 0, bytes.Length);
                return ms.ToArray();
            }
        }

        /// <summary>
        /// Decompresses a GZip-compressed byte array back to its original form.
        /// </summary>
        /// <param name="bytes">The GZip-compressed byte array to decompress.</param>
        /// <returns>The decompressed byte array in its original form.</returns>
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
