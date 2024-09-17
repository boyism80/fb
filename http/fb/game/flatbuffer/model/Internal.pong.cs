using Google.FlatBuffers;

namespace fb.game.flatbuffer.inter.model
{
    public class Pong : IFlatBufferEx
    {

        public Pong()
        { }

        public Pong(fb.game.flatbuffer.inter.Pong raw)
        {
        }

        public Pong(byte[] bytes) : this(fb.game.flatbuffer.inter.Pong.GetRootAsPong(new ByteBuffer(bytes)))
        { }

        public Offset<fb.game.flatbuffer.inter.Pong> Build(FlatBufferBuilder builder)
        {
            fb.game.flatbuffer.inter.Pong.StartPong(builder);
            return fb.game.flatbuffer.inter.Pong.EndPong(builder);
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
