using Google.FlatBuffers;

namespace fb.protocol.flatbuffer.inter.response.model
{
    public class Transfer : IFlatBufferEx
    {
        public fb.protocol.flatbuffer.inter.response.TransferResultCode Code { get; set; }
        public string Name { get; set; }
        public string Ip { get; set; }
        public ushort Port { get; set; }
        public ushort Map { get; set; }
        public ushort X { get; set; }
        public ushort Y { get; set; }
        public uint Fd { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol.flatbuffer.inter.response.Transfer raw)
        {
            Code = raw.Code;
            Name = raw.Name;
            Ip = raw.Ip;
            Port = raw.Port;
            Map = raw.Map;
            X = raw.X;
            Y = raw.Y;
            Fd = raw.Fd;
        }

        public Transfer(byte[] bytes) : this(fb.protocol.flatbuffer.inter.response.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.flatbuffer.inter.response.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.flatbuffer.inter.response.Transfer.CreateTransfer(builder,
                Code,
                builder.CreateString(Name),
                builder.CreateString(Ip),
                Port,
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
