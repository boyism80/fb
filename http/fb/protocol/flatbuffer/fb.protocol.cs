using Google.FlatBuffers;

namespace fb.protocol
{
    public class nullable_uint32
    {
        public uint Value { get; set; }

        public nullable_uint32()
        { }

        public nullable_uint32(fb.protocol.origin.nullable_uint32 raw)
        {
            Value = raw.Value;
        }


        public Offset<fb.protocol.origin.nullable_uint32> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.origin.nullable_uint32.Createnullable_uint32(builder,
                Value);
        }
    }
    public class nullable_uint16
    {
        public ushort Value { get; set; }

        public nullable_uint16()
        { }

        public nullable_uint16(fb.protocol.origin.nullable_uint16 raw)
        {
            Value = raw.Value;
        }


        public Offset<fb.protocol.origin.nullable_uint16> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.origin.nullable_uint16.Createnullable_uint16(builder,
                Value);
        }
    }
    public class nullable_bool
    {
        public bool Value { get; set; }

        public nullable_bool()
        { }

        public nullable_bool(fb.protocol.origin.nullable_bool raw)
        {
            Value = raw.Value;
        }


        public Offset<fb.protocol.origin.nullable_bool> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.origin.nullable_bool.Createnullable_bool(builder,
                Value);
        }
    }
}
