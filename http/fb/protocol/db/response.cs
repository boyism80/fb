using Google.FlatBuffers;

namespace fb.protocol.db.response
{
    public class Account : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Account;
        public string Pw { get; set; } = string.Empty;
        public ushort Map { get; set; }
        public bool Success { get; set; }

        public Account()
        { }

        public Account(fb.protocol.db.response.origin.Account raw)
        {
            Pw = raw.Pw;
            Map = raw.Map;
            Success = raw.Success;
        }

        public Account(byte[] bytes) : this(fb.protocol.db.response.origin.Account.GetRootAsAccount(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.Account> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.Account.CreateAccount(builder,
                builder.CreateString(Pw),
                Map,
                Success);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Account Deserialize(byte[] bytes)
        {
            return new Account(bytes);
        }
    }
    public class ChangePw : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ChangePw;
        public int Result { get; set; }

        public ChangePw()
        { }

        public ChangePw(fb.protocol.db.response.origin.ChangePw raw)
        {
            Result = raw.Result;
        }

        public ChangePw(byte[] bytes) : this(fb.protocol.db.response.origin.ChangePw.GetRootAsChangePw(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.ChangePw> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.ChangePw.CreateChangePw(builder,
                Result);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ChangePw Deserialize(byte[] bytes)
        {
            return new ChangePw(bytes);
        }
    }
    public class GetUid : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetUid;
        public uint Uid { get; set; }
        public bool Success { get; set; }

        public GetUid()
        { }

        public GetUid(fb.protocol.db.response.origin.GetUid raw)
        {
            Uid = raw.Uid;
            Success = raw.Success;
        }

        public GetUid(byte[] bytes) : this(fb.protocol.db.response.origin.GetUid.GetRootAsGetUid(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.GetUid> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.GetUid.CreateGetUid(builder,
                Uid,
                Success);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetUid Deserialize(byte[] bytes)
        {
            return new GetUid(bytes);
        }
    }
    public class InitCharacter : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.InitCharacter;
        public bool Success { get; set; }

        public InitCharacter()
        { }

        public InitCharacter(fb.protocol.db.response.origin.InitCharacter raw)
        {
            Success = raw.Success;
        }

        public InitCharacter(byte[] bytes) : this(fb.protocol.db.response.origin.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.InitCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.InitCharacter.CreateInitCharacter(builder,
                Success);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static InitCharacter Deserialize(byte[] bytes)
        {
            return new InitCharacter(bytes);
        }
    }
    public class MakeCharacter : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.MakeCharacter;
        public bool Success { get; set; }

        public MakeCharacter()
        { }

        public MakeCharacter(fb.protocol.db.response.origin.MakeCharacter raw)
        {
            Success = raw.Success;
        }

        public MakeCharacter(byte[] bytes) : this(fb.protocol.db.response.origin.MakeCharacter.GetRootAsMakeCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.MakeCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.MakeCharacter.CreateMakeCharacter(builder,
                Success);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static MakeCharacter Deserialize(byte[] bytes)
        {
            return new MakeCharacter(bytes);
        }
    }
    public class ReserveName : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ReserveName;
        public uint Uid { get; set; }
        public bool Success { get; set; }

        public ReserveName()
        { }

        public ReserveName(fb.protocol.db.response.origin.ReserveName raw)
        {
            Uid = raw.Uid;
            Success = raw.Success;
        }

        public ReserveName(byte[] bytes) : this(fb.protocol.db.response.origin.ReserveName.GetRootAsReserveName(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.ReserveName> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.ReserveName.CreateReserveName(builder,
                Uid,
                Success);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ReserveName Deserialize(byte[] bytes)
        {
            return new ReserveName(bytes);
        }
    }
}
