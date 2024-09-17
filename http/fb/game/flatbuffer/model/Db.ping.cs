using Google.FlatBuffers;

namespace fb.game.flatbuffer.db.model
{
    public class Ping : IFlatBufferEx
    {

        public Ping()
        { }

        public Ping(fb.game.flatbuffer.db.Ping raw)
        {
        }

        public Ping(byte[] bytes) : this(fb.game.flatbuffer.db.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.game.flatbuffer.db.Ping> Build(FlatBufferBuilder builder)
        {
            fb.game.flatbuffer.db.Ping.StartPing(builder);
            return fb.game.flatbuffer.db.Ping.EndPing(builder);
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
