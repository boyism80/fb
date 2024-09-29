using Google.FlatBuffers;

namespace fb.protocol.inter.response
{
    public class KickOut : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.KickOut;
        public uint Uid { get; set; }

        public KickOut()
        { }

        public KickOut(fb.protocol.inter.response.origin.KickOut raw)
        {
            Uid = raw.Uid;
        }

        public KickOut(byte[] bytes) : this(fb.protocol.inter.response.origin.KickOut.GetRootAsKickOut(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.response.origin.KickOut> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.response.origin.KickOut.CreateKickOut(builder,
                Uid);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static KickOut Deserialize(byte[] bytes)
        {
            return new KickOut(bytes);
        }
    }
    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public bool Success { get; set; }
        public bool Logon { get; set; }
        public string Ip { get; set; } = string.Empty;
        public ushort Port { get; set; }

        public Login()
        { }

        public Login(fb.protocol.inter.response.origin.Login raw)
        {
            Success = raw.Success;
            Logon = raw.Logon;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Login(byte[] bytes) : this(fb.protocol.inter.response.origin.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.response.origin.Login> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.response.origin.Login.CreateLogin(builder,
                Success,
                Logon,
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

        public static Login Deserialize(byte[] bytes)
        {
            return new Login(bytes);
        }
    }
    public class Logout : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Logout;
        public uint Uid { get; set; }

        public Logout()
        { }

        public Logout(fb.protocol.inter.response.origin.Logout raw)
        {
            Uid = raw.Uid;
        }

        public Logout(byte[] bytes) : this(fb.protocol.inter.response.origin.Logout.GetRootAsLogout(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.response.origin.Logout> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.response.origin.Logout.CreateLogout(builder,
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
    public class Pong : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Pong;

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
        public int ProtocolType => (int)FlatBufferProtocolType.Transfer;
        public fb.protocol.inter.TransferResult Code { get; set; }
        public string Ip { get; set; } = string.Empty;
        public ushort Port { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol.inter.response.origin.Transfer raw)
        {
            Code = (fb.protocol.inter.TransferResult)raw.Code;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Transfer(byte[] bytes) : this(fb.protocol.inter.response.origin.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.response.origin.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.response.origin.Transfer.CreateTransfer(builder,
                (fb.protocol.inter.origin.TransferResult)Code,
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

        public static Transfer Deserialize(byte[] bytes)
        {
            return new Transfer(bytes);
        }
    }
    public class Whisper : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Whisper;
        public bool Success { get; set; }
        public string From { get; set; } = string.Empty;
        public uint To { get; set; }
        public string Message { get; set; } = string.Empty;

        public Whisper()
        { }

        public Whisper(fb.protocol.inter.response.origin.Whisper raw)
        {
            Success = raw.Success;
            From = raw.From;
            To = raw.To;
            Message = raw.Message;
        }

        public Whisper(byte[] bytes) : this(fb.protocol.inter.response.origin.Whisper.GetRootAsWhisper(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.inter.response.origin.Whisper> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.inter.response.origin.Whisper.CreateWhisper(builder,
                Success,
                builder.CreateString(From),
                To,
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
