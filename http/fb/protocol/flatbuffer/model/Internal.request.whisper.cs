using Google.FlatBuffers;

namespace fb.protocol.flatbuffer.inter.request.model
{
    public class Whisper : IFlatBufferEx
    {
        public string Name { get; set; }
        public string Message { get; set; }

        public Whisper()
        { }

        public Whisper(fb.protocol.flatbuffer.inter.request.Whisper raw)
        {
            Name = raw.Name;
            Message = raw.Message;
        }

        public Whisper(byte[] bytes) : this(fb.protocol.flatbuffer.inter.request.Whisper.GetRootAsWhisper(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.flatbuffer.inter.request.Whisper> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.flatbuffer.inter.request.Whisper.CreateWhisper(builder,
                builder.CreateString(Name),
                builder.CreateString(Message));
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Whisper Deserialize(byte[] bytes)
        {
            return new Whisper(bytes);
        }
    }
}
