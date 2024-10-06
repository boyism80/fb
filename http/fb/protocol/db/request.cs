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
    public class DeleteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.DeleteArticle;
        public uint Id { get; set; }
        public uint User { get; set; }

        public DeleteArticle()
        { }

        public DeleteArticle(fb.protocol.db.request.origin.DeleteArticle raw)
        {
            Id = raw.Id;
            User = raw.User;
        }

        public DeleteArticle(byte[] bytes) : this(fb.protocol.db.request.origin.DeleteArticle.GetRootAsDeleteArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.DeleteArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.DeleteArticle.CreateDeleteArticle(builder,
                Id,
                User);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static DeleteArticle Deserialize(byte[] bytes)
        {
            return new DeleteArticle(bytes);
        }
    }
    public class GetArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetArticle;
        public uint Section { get; set; }
        public uint Article { get; set; }

        public GetArticle()
        { }

        public GetArticle(fb.protocol.db.request.origin.GetArticle raw)
        {
            Section = raw.Section;
            Article = raw.Article;
        }

        public GetArticle(byte[] bytes) : this(fb.protocol.db.request.origin.GetArticle.GetRootAsGetArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.GetArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.GetArticle.CreateGetArticle(builder,
                Section,
                Article);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetArticle Deserialize(byte[] bytes)
        {
            return new GetArticle(bytes);
        }
    }
    public class GetArticleList : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetArticleList;
        public uint Section { get; set; }
        public uint Position { get; set; }

        public GetArticleList()
        { }

        public GetArticleList(fb.protocol.db.request.origin.GetArticleList raw)
        {
            Section = raw.Section;
            Position = raw.Position;
        }

        public GetArticleList(byte[] bytes) : this(fb.protocol.db.request.origin.GetArticleList.GetRootAsGetArticleList(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.GetArticleList> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.GetArticleList.CreateGetArticleList(builder,
                Section,
                Position);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetArticleList Deserialize(byte[] bytes)
        {
            return new GetArticleList(bytes);
        }
    }
    public class WriteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.WriteArticle;
        public uint Section { get; set; }
        public uint User { get; set; }
        public string Title { get; set; } = string.Empty;
        public string Contents { get; set; } = string.Empty;

        public WriteArticle()
        { }

        public WriteArticle(fb.protocol.db.request.origin.WriteArticle raw)
        {
            Section = raw.Section;
            User = raw.User;
            Title = raw.Title;
            Contents = raw.Contents;
        }

        public WriteArticle(byte[] bytes) : this(fb.protocol.db.request.origin.WriteArticle.GetRootAsWriteArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.WriteArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.WriteArticle.CreateWriteArticle(builder,
                Section,
                User,
                builder.CreateString(Title),
                builder.CreateString(Contents));
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static WriteArticle Deserialize(byte[] bytes)
        {
            return new WriteArticle(bytes);
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
    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public uint Uid { get; set; }

        public Login()
        { }

        public Login(fb.protocol.db.request.origin.Login raw)
        {
            Uid = raw.Uid;
        }

        public Login(byte[] bytes) : this(fb.protocol.db.request.origin.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.Login> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.Login.CreateLogin(builder,
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
    public class Save : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Save;
        public fb.protocol.db.Character Character { get; set; }
        public List<fb.protocol.db.Item> Items { get; set; }
        public List<fb.protocol.db.Spell> Spells { get; set; }

        public Save()
        { }

        public Save(fb.protocol.db.request.origin.Save raw)
        {
            Character = new Character(raw.Character.Value);
            Items = Enumerable.Range(0, raw.ItemsLength).Select(i => new Item(raw.Items(i).Value)).ToList();
            Spells = Enumerable.Range(0, raw.SpellsLength).Select(i => new Spell(raw.Spells(i).Value)).ToList();
        }

        public Save(byte[] bytes) : this(fb.protocol.db.request.origin.Save.GetRootAsSave(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.origin.Save> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.origin.Save.CreateSave(builder,
                Character.Build(builder),
                fb.protocol.db.request.origin.Save.CreateItemsVector(builder, Items.ConvertAll(x => x.Build(builder)).ToArray()),
                fb.protocol.db.request.origin.Save.CreateSpellsVector(builder, Spells.ConvertAll(x => x.Build(builder)).ToArray()));
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Save Deserialize(byte[] bytes)
        {
            return new Save(bytes);
        }
    }
}
