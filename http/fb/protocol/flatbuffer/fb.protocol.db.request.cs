using Google.FlatBuffers;

namespace fb.protocol.db.request
{
    public class CreateCharacter : IFlatBufferEx
    {
        public string Name { get; set; } = string.Empty;

        public CreateCharacter()
        { }

        public CreateCharacter(fb.protocol.db.request.origin.CreateCharacter raw)
        {
            Name = raw.Name;
        }

        public CreateCharacter(byte[] bytes) : this(fb.protocol.db.request.origin.CreateCharacter.GetRootAsCreateCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.CreateCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.CreateCharacter.CreateCreateCharacter(builder,
                builder.CreateString(Name));
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static CreateCharacter Deserialize(byte[] bytes)
        {
            return new CreateCharacter(bytes);
        }
    }
    public class FinishCharacter : IFlatBufferEx
    {
        public uint Pk { get; set; }
        public byte Hair { get; set; }
        public byte Sex { get; set; }
        public byte Nation { get; set; }
        public byte Creature { get; set; }

        public FinishCharacter()
        { }

        public FinishCharacter(fb.protocol.db.request.origin.FinishCharacter raw)
        {
            Pk = raw.Pk;
            Hair = raw.Hair;
            Sex = raw.Sex;
            Nation = raw.Nation;
            Creature = raw.Creature;
        }

        public FinishCharacter(byte[] bytes) : this(fb.protocol.db.request.origin.FinishCharacter.GetRootAsFinishCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.FinishCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.FinishCharacter.CreateFinishCharacter(builder,
                Pk,
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

        public static FinishCharacter Deserialize(byte[] bytes)
        {
            return new FinishCharacter(bytes);
        }
    }
    public class InitCharacter : IFlatBufferEx
    {
        public uint Pk { get; set; }
        public string Name { get; set; } = string.Empty;
        public string Pw { get; set; } = string.Empty;
        public uint Hp { get; set; }
        public uint Mp { get; set; }
        public uint Map { get; set; }
        public ushort X { get; set; }
        public ushort Y { get; set; }
        public bool Admin { get; set; }

        public InitCharacter()
        { }

        public InitCharacter(fb.protocol.db.request.origin.InitCharacter raw)
        {
            Pk = raw.Pk;
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
                Pk,
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
    public class Ping : IFlatBufferEx
    {

        public Ping()
        { }

        public Ping(fb.protocol.db.request.origin.Ping raw)
        {
        }

        public Ping(byte[] bytes) : this(fb.protocol.db.request.origin.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.Ping> Build(FlatBufferBuilder builder)
        {
            fb.protocol.db.request.origin.Ping.StartPing(builder);
            return fb.protocol.db.request.origin.Ping.EndPing(builder);
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
}
