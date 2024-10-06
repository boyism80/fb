using Google.FlatBuffers;

namespace fb.protocol.db.response
{
    public class Account : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Account;
        public string Pw { get; set; } = string.Empty;
        public uint Map { get; set; }
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
    public class DeleteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.DeleteArticle;
        public int Result { get; set; }

        public DeleteArticle()
        { }

        public DeleteArticle(fb.protocol.db.response.origin.DeleteArticle raw)
        {
            Result = raw.Result;
        }

        public DeleteArticle(byte[] bytes) : this(fb.protocol.db.response.origin.DeleteArticle.GetRootAsDeleteArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.DeleteArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.DeleteArticle.CreateDeleteArticle(builder,
                Result);
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
        public fb.protocol.db.Article Article { get; set; }
        public bool Next { get; set; }
        public bool Success { get; set; }

        public GetArticle()
        { }

        public GetArticle(fb.protocol.db.response.origin.GetArticle raw)
        {
            Article = new Article(raw.Article.Value);
            Next = raw.Next;
            Success = raw.Success;
        }

        public GetArticle(byte[] bytes) : this(fb.protocol.db.response.origin.GetArticle.GetRootAsGetArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.GetArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.GetArticle.CreateGetArticle(builder,
                Article.Build(builder),
                Next,
                Success);
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
        public List<fb.protocol.db.ArticleSummary> SummaryList { get; set; }

        public GetArticleList()
        { }

        public GetArticleList(fb.protocol.db.response.origin.GetArticleList raw)
        {
            SummaryList = Enumerable.Range(0, raw.SummaryListLength).Select(i => new ArticleSummary(raw.SummaryList(i).Value)).ToList();
        }

        public GetArticleList(byte[] bytes) : this(fb.protocol.db.response.origin.GetArticleList.GetRootAsGetArticleList(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.GetArticleList> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.GetArticleList.CreateGetArticleList(builder,
                fb.protocol.db.response.origin.GetArticleList.CreateSummaryListVector(builder, SummaryList.ConvertAll(x => x.Build(builder)).ToArray()));
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
        public bool Success { get; set; }

        public WriteArticle()
        { }

        public WriteArticle(fb.protocol.db.response.origin.WriteArticle raw)
        {
            Success = raw.Success;
        }

        public WriteArticle(byte[] bytes) : this(fb.protocol.db.response.origin.WriteArticle.GetRootAsWriteArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.WriteArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.WriteArticle.CreateWriteArticle(builder,
                Success);
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
    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public fb.protocol.db.Character Character { get; set; }
        public List<fb.protocol.db.Item> Items { get; set; }
        public List<fb.protocol.db.Spell> Spells { get; set; }

        public Login()
        { }

        public Login(fb.protocol.db.response.origin.Login raw)
        {
            Character = new Character(raw.Character.Value);
            Items = Enumerable.Range(0, raw.ItemsLength).Select(i => new Item(raw.Items(i).Value)).ToList();
            Spells = Enumerable.Range(0, raw.SpellsLength).Select(i => new Spell(raw.Spells(i).Value)).ToList();
        }

        public Login(byte[] bytes) : this(fb.protocol.db.response.origin.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.Login> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.Login.CreateLogin(builder,
                Character.Build(builder),
                fb.protocol.db.response.origin.Login.CreateItemsVector(builder, Items.ConvertAll(x => x.Build(builder)).ToArray()),
                fb.protocol.db.response.origin.Login.CreateSpellsVector(builder, Spells.ConvertAll(x => x.Build(builder)).ToArray()));
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
    public class Save : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Save;
        public bool Success { get; set; }

        public Save()
        { }

        public Save(fb.protocol.db.response.origin.Save raw)
        {
            Success = raw.Success;
        }

        public Save(byte[] bytes) : this(fb.protocol.db.response.origin.Save.GetRootAsSave(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.origin.Save> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.origin.Save.CreateSave(builder,
                Success);
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
