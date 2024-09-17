using Google.FlatBuffers;

namespace fb.protocol.flatbuffer.inter.response.model
{
    public class Pong : IFlatBufferEx
    {

        public Pong()
        { }

        public Pong(fb.protocol.flatbuffer.inter.response.Pong raw)
        {
        }

        public Pong(byte[] bytes) : this(fb.protocol.flatbuffer.inter.response.Pong.GetRootAsPong(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.flatbuffer.inter.response.Pong> Build(FlatBufferBuilder builder)
        {
            fb.protocol.flatbuffer.inter.response.Pong.StartPong(builder);
            return fb.protocol.flatbuffer.inter.response.Pong.EndPong(builder);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Pong Deserialize(byte[] bytes)
        {
            return new Pong(bytes);
        }
    }
}
