using Google.FlatBuffers;

namespace fb.protocol.db.request
{
    public class Account : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Account;
        public uint Uid { get; set; }

        public Account()
        { }

        public Account(fb.protocol.db.request.origin.Account raw)
        {
            Uid = raw.Uid;
        }

        public Account(byte[] bytes) : this(fb.protocol.db.request.origin.Account.GetRootAsAccount(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.Account> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.Account.CreateAccount(builder,
                Uid);
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
        public uint Uid { get; set; }
        public string Before { get; set; } = string.Empty;
        public string After { get; set; } = string.Empty;
        public uint Birthday { get; set; }

        public ChangePw()
        { }

        public ChangePw(fb.protocol.db.request.origin.ChangePw raw)
        {
            Uid = raw.Uid;
            Before = raw.Before;
            After = raw.After;
            Birthday = raw.Birthday;
        }

        public ChangePw(byte[] bytes) : this(fb.protocol.db.request.origin.ChangePw.GetRootAsChangePw(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.ChangePw> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.ChangePw.CreateChangePw(builder,
                Uid,
                builder.CreateString(Before),
                builder.CreateString(After),
                Birthday);
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
    public class InitCharacter : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.InitCharacter;
        public uint Uid { get; set; }
        public string Name { get; set; } = string.Empty;
        public string Pw { get; set; } = string.Empty;
        public uint Hp { get; set; }
        public uint Mp { get; set; }
        public ushort Map { get; set; }
        public ushort X { get; set; }
        public ushort Y { get; set; }
        public bool Admin { get; set; }

        public InitCharacter()
        { }

        public InitCharacter(fb.protocol.db.request.origin.InitCharacter raw)
        {
            Uid = raw.Uid;
            Name = raw.Name;
            Pw = raw.Pw;
            Hp = raw.Hp;
            Mp = raw.Mp;
            Map = raw.Map;
            X = raw.X;
            Y = raw.Y;
            Admin = raw.Admin;
        }

        public InitCharacter(byte[] bytes) : this(fb.protocol.db.request.origin.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.InitCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.InitCharacter.CreateInitCharacter(builder,
                Uid,
                builder.CreateString(Name),
                builder.CreateString(Pw),
                Hp,
                Mp,
                Map,
                X,
                Y,
                Admin);
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
        public uint Uid { get; set; }
        public ushort Hair { get; set; }
        public byte Sex { get; set; }
        public byte Nation { get; set; }
        public byte Creature { get; set; }

        public MakeCharacter()
        { }

        public MakeCharacter(fb.protocol.db.request.origin.MakeCharacter raw)
        {
            Uid = raw.Uid;
            Hair = raw.Hair;
            Sex = raw.Sex;
            Nation = raw.Nation;
            Creature = raw.Creature;
        }

        public MakeCharacter(byte[] bytes) : this(fb.protocol.db.request.origin.MakeCharacter.GetRootAsMakeCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.MakeCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.MakeCharacter.CreateMakeCharacter(builder,
                Uid,
                Hair,
                Sex,
                Nation,
                Creature);
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
        public string Name { get; set; } = string.Empty;

        public ReserveName()
        { }

        public ReserveName(fb.protocol.db.request.origin.ReserveName raw)
        {
            Name = raw.Name;
        }

        public ReserveName(byte[] bytes) : this(fb.protocol.db.request.origin.ReserveName.GetRootAsReserveName(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.ReserveName> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.ReserveName.CreateReserveName(builder,
                builder.CreateString(Name));
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
