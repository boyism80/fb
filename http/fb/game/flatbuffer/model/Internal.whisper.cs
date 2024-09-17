using Google.FlatBuffers;

namespace fb.game.flatbuffer.inter.model
{
    public class Whisper : IFlatBufferEx
    {
        public string Name { get; set; }
        public string Message { get; set; }

        public Whisper()
        { }

        public Whisper(fb.game.flatbuffer.inter.Whisper raw)
        {
            Name = raw.Name;
            Message = raw.Message;
        }

        public Whisper(byte[] bytes) : this(fb.game.flatbuffer.inter.Whisper.GetRootAsWhisper(new ByteBuffer(bytes)))
        { }

        public Offset<fb.game.flatbuffer.inter.Whisper> Build(FlatBufferBuilder builder)
        {
            return fb.game.flatbuffer.inter.Whisper.CreateWhisper(builder,
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
