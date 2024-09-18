using Google.FlatBuffers;

namespace fb.protocol.inter.response
{
    public enum TransferResultCode : sbyte
    {
        CONNECTED = fb.protocol.inter.response.origin.TransferResultCode.CONNECTED,
        SUCCESS = fb.protocol.inter.response.origin.TransferResultCode.SUCCESS
    }

    public class Pong : IFlatBufferEx
    {

        public Pong()
        { }

        public Pong(fb.protocol.inter.response.origin.Pong raw)
        {
        }

        public Pong(byte[] bytes) : this(fb.protocol.inter.response.origin.Pong.GetRootAsPong(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.response.origin.Pong> Build(FlatBufferBuilder builder)
        {
            fb.protocol.inter.response.origin.Pong.StartPong(builder);
            return fb.protocol.inter.response.origin.Pong.EndPong(builder);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Pong Deserialize(byte[] bytes)
        {
            return new Pong(bytes);
        }
    }
    public class Transfer : IFlatBufferEx
    {
        public TransferResultCode Code { get; set; }
        public string Name { get; set; }
        public string Ip { get; set; }
        public ushort Port { get; set; }
        public ushort Map { get; set; }
        public ushort X { get; set; }
        public ushort Y { get; set; }
        public uint Fd { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol.inter.response.origin.Transfer raw)
        {
            Code = (fb.protocol.inter.response.TransferResultCode)raw.Code;
            Name = raw.Name;
            Ip = raw.Ip;
            Port = raw.Port;
            Map = raw.Map;
            X = raw.X;
            Y = raw.Y;
            Fd = raw.Fd;
        }

        public Transfer(byte[] bytes) : this(fb.protocol.inter.response.origin.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.response.origin.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.response.origin.Transfer.CreateTransfer(builder,
                (fb.protocol.inter.response.origin.TransferResultCode)Code,
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
