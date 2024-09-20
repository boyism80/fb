using Google.FlatBuffers;

namespace fb.protocol.inter.request
{
    public class Login : IFlatBufferEx
    {
        public uint Uid { get; set; }

        public Login()
        { }

        public Login(fb.protocol.inter.request.origin.Login raw)
        {
            Uid = raw.Uid;
        }

        public Login(byte[] bytes) : this(fb.protocol.inter.request.origin.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.request.origin.Login> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Login.CreateLogin(builder,
                Uid);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Login Deserialize(byte[] bytes)
        {
            return new Login(bytes);
        }
    }
    public class Logout : IFlatBufferEx
    {
        public uint Uid { get; set; }

        public Logout()
        { }

        public Logout(fb.protocol.inter.request.origin.Logout raw)
        {
            Uid = raw.Uid;
        }

        public Logout(byte[] bytes) : this(fb.protocol.inter.request.origin.Logout.GetRootAsLogout(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.request.origin.Logout> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Logout.CreateLogout(builder,
                Uid);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Logout Deserialize(byte[] bytes)
        {
            return new Logout(bytes);
        }
    }
    public class Ping : IFlatBufferEx
    {
        public string Name { get; set; } = string.Empty;
        public fb.protocol.inter.Service Service { get; set; }
        public byte Group { get; set; }
        public string Ip { get; set; } = string.Empty;
        public ushort Port { get; set; }

        public Ping()
        { }

        public Ping(fb.protocol.inter.request.origin.Ping raw)
        {
            Name = raw.Name;
            Service = (fb.protocol.inter.Service)raw.Service;
            Group = raw.Group;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Ping(byte[] bytes) : this(fb.protocol.inter.request.origin.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.request.origin.Ping> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Ping.CreatePing(builder,
                builder.CreateString(Name),
                (fb.protocol.inter.origin.Service)Service,
                Group,
                builder.CreateString(Ip),
                Port);
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
        public fb.protocol.inter.Service Service { get; set; }
        public byte Group { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol.inter.request.origin.Transfer raw)
        {
            Service = (fb.protocol.inter.Service)raw.Service;
            Group = raw.Group;
        }

        public Transfer(byte[] bytes) : this(fb.protocol.inter.request.origin.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.request.origin.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.request.origin.Transfer.CreateTransfer(builder,
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

        public static Transfer Deserialize(byte[] bytes)
        {
            return new Transfer(bytes);
        }
    }
    public class Whisper : IFlatBufferEx
    {
        public string Name { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;

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
