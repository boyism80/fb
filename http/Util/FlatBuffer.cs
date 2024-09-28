using Google.FlatBuffers;

namespace http.Util
{
    public static class FlatBuffer
    {
        public static byte[] ToBytes(this IFlatBufferEx flatbuffer)
        {
            using var ms = new MemoryStream();
            using (var writer = new BinaryWriter(ms))
            {
                var bytes = flatbuffer.Serialize();
                writer.Write(flatbuffer.ProtocolType.ToMachineEndian());
                writer.Write(bytes.Length.ToMachineEndian());
                writer.Write(bytes);
                writer.Flush();
            }

            return ms.ToArray();
        }
    }
}
