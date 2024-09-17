using Google.FlatBuffers;

namespace fb.protocol.inter.response
{
    public enum TransferResultCode : sbyte
    {
        CONNECTED = fb.protocol.inter.response.origin.CONNECTED,
        SUCCESS = fb.protocol.inter.response.origin.SUCCESS
    }

    public class Pong : IFlatBufferEx
    {

        public Pong()
        { }

        public Pong(fb.protocol.inter.response.origin.Pong raw)
        {
        }


        public Offset<fb.protocol.inter.response.origin.Pong> Build(FlatBufferBuilder builder)
        {
            fb.protocol.inter.response.origin.Pong.StartPong(builder);
            return fb.protocol.inter.response.origin.Pong.EndPong(builder);
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
            Code = raw.Code;
            Name = raw.Name;
            Ip = raw.Ip;
            Port = raw.Port;
            Map = raw.Map;
            X = raw.X;
            Y = raw.Y;
            Fd = raw.Fd;
        }


        public Offset<fb.protocol.inter.response.origin.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.response.origin.Transfer.CreateTransfer(builder,
                Code,
                builder.CreateString(Name),
                builder.CreateString(Ip),
                Port,
                Map,
                X,
                Y,
                Fd);
        }
    }
}
