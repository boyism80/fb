using Google.FlatBuffers;

namespace fb.protocol.db
{
    public class nullable_uint
    {
        public uint Value { get; set; }

        public nullable_uint()
        { }

        public nullable_uint(fb.protocol.db.origin.nullable_uint raw)
        {
            Value = raw.Value;
        }


        public Offset<fb.protocol.db.origin.nullable_uint> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.nullable_uint.Createnullable_uint(builder,
                Value);
        }
    }
}
