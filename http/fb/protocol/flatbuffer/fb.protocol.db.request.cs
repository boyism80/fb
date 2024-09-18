using Google.FlatBuffers;

namespace fb.protocol.db.request
{
    public class Ping : IFlatBufferEx
    {

        public Ping()
        { }

        public Ping(fb.protocol.db.request.origin.Ping raw)
        {
        }

        public Ping(byte[] bytes) : this(fb.protocol.db.request.origin.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.Ping> Build(FlatBufferBuilder builder)
        {
            fb.protocol.db.request.origin.Ping.StartPing(builder);
            return fb.protocol.db.request.origin.Ping.EndPing(builder);
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
