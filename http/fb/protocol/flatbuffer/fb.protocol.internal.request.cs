using Google.FlatBuffers;

namespace fb.protocol.inter.request
{
    public enum Service : sbyte
    {
        Gateway = fb.protocol.inter.request.origin.Gateway,
        Login = fb.protocol.inter.request.origin.Login,
        Game = fb.protocol.inter.request.origin.Game
    }

    public class Ping : IFlatBufferEx
    {

        public Ping()
        { }

        public Ping(fb.protocol.inter.request.origin.Ping raw)
        {
        }


        public Offset<fb.protocol.inter.request.origin.Ping> Build(FlatBufferBuilder builder)
        {
            fb.protocol.inter.request.origin.Ping.StartPing(builder);
            return fb.protocol.inter.request.origin.Ping.EndPing(builder);
        }
    }
    public class Transfer : IFlatBufferEx
    {
        public string Name { get; set; }
        public Service From { get; set; }
        public Service To { get; set; }
        public ushort Map { get; set; }
        public ushort X { get; set; }
        public ushort Y { get; set; }
        public uint Fd { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol.inter.request.origin.Transfer raw)
        {
            Name = raw.Name;
            From = raw.From;
            To = raw.To;
            Map = raw.Map;
            X = raw.X;
            Y = raw.Y;
            Fd = raw.Fd;
        }


        public Offset<fb.protocol.inter.request.origin.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Transfer.CreateTransfer(builder,
                builder.CreateString(Name),
                From,
                To,
                Map,
                X,
                Y,
                Fd);
        }
    }
    public class Whisper : IFlatBufferEx
    {
        public string Name { get; set; }
        public string Message { get; set; }

        public Whisper()
        { }

        public Whisper(fb.protocol.inter.request.origin.Whisper raw)
        {
            Name = raw.Name;
            Message = raw.Message;
        }


        public Offset<fb.protocol.inter.request.origin.Whisper> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Whisper.CreateWhisper(builder,
                builder.CreateString(Name),
                builder.CreateString(Message));
        }
    }
}
