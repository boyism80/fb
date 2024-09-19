using Google.FlatBuffers;

namespace fb.protocol.db.response
{
    public class CreateCharacter : IFlatBufferEx
    {
        public uint Pk { get; set; }
        public bool Success { get; set; }

        public CreateCharacter()
        { }

        public CreateCharacter(fb.protocol.db.response.origin.CreateCharacter raw)
        {
            Pk = raw.Pk;
            Success = raw.Success;
        }

        public CreateCharacter(byte[] bytes) : this(fb.protocol.db.response.origin.CreateCharacter.GetRootAsCreateCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.CreateCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.CreateCharacter.CreateCreateCharacter(builder,
                Pk,
                Success);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static CreateCharacter Deserialize(byte[] bytes)
        {
            return new CreateCharacter(bytes);
        }
    }
    public class FinishCharacter : IFlatBufferEx
    {
        public bool Success { get; set; }

        public FinishCharacter()
        { }

        public FinishCharacter(fb.protocol.db.response.origin.FinishCharacter raw)
        {
            Success = raw.Success;
        }

        public FinishCharacter(byte[] bytes) : this(fb.protocol.db.response.origin.FinishCharacter.GetRootAsFinishCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.FinishCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.FinishCharacter.CreateFinishCharacter(builder,
                Success);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static FinishCharacter Deserialize(byte[] bytes)
        {
            return new FinishCharacter(bytes);
        }
    }
    public class InitCharacter : IFlatBufferEx
    {
        public bool Success { get; set; }

        public InitCharacter()
        { }

        public InitCharacter(fb.protocol.db.response.origin.InitCharacter raw)
        {
            Success = raw.Success;
        }

        public InitCharacter(byte[] bytes) : this(fb.protocol.db.response.origin.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.InitCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.InitCharacter.CreateInitCharacter(builder,
                Success);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static InitCharacter Deserialize(byte[] bytes)
        {
            return new InitCharacter(bytes);
        }
    }
}
