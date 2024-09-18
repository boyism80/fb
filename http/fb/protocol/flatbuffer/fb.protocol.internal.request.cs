using Google.FlatBuffers;

namespace fb.protocol.inter.request
{
    public class Ping : IFlatBufferEx
    {
        public string Name { get; set; }
        public fb.protocol.inter.Service Service { get; set; }
        public byte Group { get; set; }

        public Ping()
        { }

        public Ping(fb.protocol.inter.request.origin.Ping raw)
        {
            Name = raw.Name;
            Service = (fb.protocol.inter.Service)raw.Service;
            Group = raw.Group;
        }

        public Ping(byte[] bytes) : this(fb.protocol.inter.request.origin.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.request.origin.Ping> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Ping.CreatePing(builder,
                builder.CreateString(Name),
                (fb.protocol.inter.origin.Service)Service,
                Group);
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
        public fb.protocol.inter.Service From { get; set; }
        public fb.protocol.inter.Service To { get; set; }
        public ushort Map { get; set; }
        public ushort X { get; set; }
        public ushort Y { get; set; }
        public uint Fd { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol.inter.request.origin.Transfer raw)
        {
            Name = raw.Name;
            From = (fb.protocol.inter.Service)raw.From;
            To = (fb.protocol.inter.Service)raw.To;
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
                (fb.protocol.inter.origin.Service)From,
                (fb.protocol.inter.origin.Service)To,
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
