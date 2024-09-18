using Google.FlatBuffers;

namespace fb.protocol.inter.request
{
    public enum Service : sbyte
    {
        Gateway = fb.protocol.inter.request.origin.Service.Gateway,
        Login = fb.protocol.inter.request.origin.Service.Login,
        Game = fb.protocol.inter.request.origin.Service.Game
    }

    public class Ping : IFlatBufferEx
    {

        public Ping()
        { }

        public Ping(fb.protocol.inter.request.origin.Ping raw)
        {
        }

        public Ping(byte[] bytes) : this(fb.protocol.inter.request.origin.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.request.origin.Ping> Build(FlatBufferBuilder builder)
        {
            fb.protocol.inter.request.origin.Ping.StartPing(builder);
            return fb.protocol.inter.request.origin.Ping.EndPing(builder);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Ping Deserialize(byte[] bytes)
        {
            return new Ping(bytes);
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
            From = (fb.protocol.inter.request.Service)raw.From;
            To = (fb.protocol.inter.request.Service)raw.To;
            Map = raw.Map;
            X = raw.X;
            Y = raw.Y;
            Fd = raw.Fd;
        }

        public Transfer(byte[] bytes) : this(fb.protocol.inter.request.origin.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.request.origin.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Transfer.CreateTransfer(builder,
                builder.CreateString(Name),
                (fb.protocol.inter.request.origin.Service)From,
                (fb.protocol.inter.request.origin.Service)To,
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

        public Whisper(byte[] bytes) : this(fb.protocol.inter.request.origin.Whisper.GetRootAsWhisper(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.request.origin.Whisper> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Whisper.CreateWhisper(builder,
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
