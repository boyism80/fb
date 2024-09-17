using Google.FlatBuffers;

namespace fb.protocol.flatbuffer.inter.request.model
{
    public class Ping : IFlatBufferEx
    {

        public Ping()
        { }

        public Ping(fb.protocol.flatbuffer.inter.request.Ping raw)
        {
        }

        public Ping(byte[] bytes) : this(fb.protocol.flatbuffer.inter.request.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.flatbuffer.inter.request.Ping> Build(FlatBufferBuilder builder)
        {
            fb.protocol.flatbuffer.inter.request.Ping.StartPing(builder);
            return fb.protocol.flatbuffer.inter.request.Ping.EndPing(builder);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Ping Deserialize(byte[] bytes)
        {
            return new Ping(bytes);
        }
    }
}
