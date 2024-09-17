using Google.FlatBuffers;

namespace fb.protocol.flatbuffer.inter.request.model
{
    public class Transfer : IFlatBufferEx
    {
        public string Name { get; set; }
        public fb.protocol.flatbuffer.inter.request.Service From { get; set; }
        public fb.protocol.flatbuffer.inter.request.Service To { get; set; }
        public ushort Map { get; set; }
        public ushort X { get; set; }
        public ushort Y { get; set; }
        public uint Fd { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol.flatbuffer.inter.request.Transfer raw)
        {
            Name = raw.Name;
            From = raw.From;
            To = raw.To;
            Map = raw.Map;
            X = raw.X;
            Y = raw.Y;
            Fd = raw.Fd;
        }

        public Transfer(byte[] bytes) : this(fb.protocol.flatbuffer.inter.request.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.flatbuffer.inter.request.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.flatbuffer.inter.request.Transfer.CreateTransfer(builder,
                builder.CreateString(Name),
                From,
                To,
                Map,
                X,
                Y,
                Fd);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Transfer Deserialize(byte[] bytes)
        {
            return new Transfer(bytes);
        }
    }
}
