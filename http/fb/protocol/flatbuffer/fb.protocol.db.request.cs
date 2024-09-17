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


        public Offset<fb.protocol.db.request.origin.Ping> Build(FlatBufferBuilder builder)
        {
            fb.protocol.db.request.origin.Ping.StartPing(builder);
            return fb.protocol.db.request.origin.Ping.EndPing(builder);
        }
    }
}
