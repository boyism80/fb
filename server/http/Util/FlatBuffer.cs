using Google.FlatBuffers;

namespace Http.Util
{
    /// <summary>
    /// Provides utility methods for FlatBuffer serialization and conversion operations.
    /// Contains extension methods for converting FlatBuffer objects to byte arrays with protocol headers.
    /// </summary>
    public static class FlatBuffer
    {
        /// <summary>
        /// Converts a FlatBuffer object to a byte array with protocol type and length headers.
        /// The resulting byte array contains the protocol type, data length, and serialized data.
        /// </summary>
        /// <param name="flatbuffer">The FlatBuffer object to convert to bytes.</param>
        /// <returns>A byte array containing the protocol type (4 bytes), data length (4 bytes), and serialized data.</returns>
        public static byte[] ToBytes(this IFlatBufferEx flatbuffer)
        {
            using var ms = new MemoryStream();
            using (var writer = new BinaryWriter(ms))
            {
                var bytes = flatbuffer.Serialize();
                writer.Write(flatbuffer.ProtocolType);
                writer.Write(bytes.Length);
                writer.Write(bytes);
                writer.Flush();
            }

            return ms.ToArray();
        }
    }
}
