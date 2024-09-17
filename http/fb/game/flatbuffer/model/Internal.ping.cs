using Google.FlatBuffers;

namespace fb.game.flatbuffer.inter.model
{
    public class Ping : IFlatBufferEx
    {

        public Ping()
        { }

        public Ping(fb.game.flatbuffer.inter.Ping raw)
        {
        }

        public Ping(byte[] bytes) : this(fb.game.flatbuffer.inter.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.game.flatbuffer.inter.Ping> Build(FlatBufferBuilder builder)
        {
            fb.game.flatbuffer.inter.Ping.StartPing(builder);
            return fb.game.flatbuffer.inter.Ping.EndPing(builder);
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
