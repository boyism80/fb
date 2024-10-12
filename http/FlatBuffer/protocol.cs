using Google.FlatBuffers;
using nullable;

namespace Google.FlatBuffers
{
    public interface IFlatBufferEx
    {
        int ProtocolType { get; }
        byte[] Serialize();
    }
}

namespace fb.protocol.db
{
    public enum FlatBufferProtocolType
    { 
        Position,
        Character,
        Item,
        Spell,
        ArticleSummary,
        Article
    }
}
namespace fb.protocol.db.request
{
    public enum FlatBufferProtocolType
    { 
        Account,
        ChangePw,
        InitCharacter,
        Login,
        MakeCharacter,
        ReserveName,
        Save,
        GetArticle,
        GetArticleList,
        WriteArticle,
        DeleteArticle
    }
}
namespace fb.protocol.db.response
{
    public enum FlatBufferProtocolType
    { 
        Account,
        DeleteArticle,
        GetArticle,
        GetArticleList,
        WriteArticle,
        ChangePw,
        GetUid,
        InitCharacter,
        Login,
        MakeCharacter,
        ReserveName,
        Save
    }
}
namespace fb.protocol._internal
{
    public enum FlatBufferProtocolType
    { 
    }
}
namespace fb.protocol._internal.request
{
    public enum FlatBufferProtocolType
    { 
        Login,
        Logout,
        Ping,
        Transfer,
        Whisper,
        KickOut
    }
}
namespace fb.protocol._internal.response
{
    public enum FlatBufferProtocolType
    { 
        KickOut,
        Login,
        Logout,
        Pong,
        Transfer,
        Whisper
    }
}

namespace fb.protocol.db
{

    public class Position : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Position;
        public ushort X { get; set; }
        public ushort Y { get; set; }

        public Position()
        { }

        public Position(fb.protocol.db.raw.Position raw)
        {
            X = raw.X;
            Y = raw.Y;
        }

        public Position(byte[] bytes) : this(fb.protocol.db.raw.Position.GetRootAsPosition(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.raw.Position> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.raw.Position.CreatePosition(builder,
                X,
                Y);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Position Deserialize(byte[] bytes)
        {
            return new Position(bytes);
        }
    }
    public class Character : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Character;
        public uint Id { get; set; }
        public string Name { get; set; }
        public string LastLogin { get; set; }
        public bool Admin { get; set; }
        public ushort Look { get; set; }
        public ushort Color { get; set; }
        public ushort Sex { get; set; }
        public ushort Nation { get; set; }
        public ushort? Creature { get; set; }
        public uint Map { get; set; }
        public fb.protocol.db.Position Position { get; set; }
        public byte Direction { get; set; }
        public byte State { get; set; }
        public byte ClassType { get; set; }
        public byte Promotion { get; set; }
        public uint Exp { get; set; }
        public uint Money { get; set; }
        public uint DepositedMoney { get; set; }
        public ushort? Disguise { get; set; }
        public uint Hp { get; set; }
        public uint BaseHp { get; set; }
        public uint AdditionalHp { get; set; }
        public uint Mp { get; set; }
        public uint BaseMp { get; set; }
        public uint AdditionalMp { get; set; }
        public byte? WeaponColor { get; set; }
        public byte? HelmetColor { get; set; }
        public byte? ArmorColor { get; set; }
        public byte? ShieldColor { get; set; }
        public byte? RingLeftColor { get; set; }
        public byte? RingRightColor { get; set; }
        public byte? AuxTopColor { get; set; }
        public byte? AuxBotColor { get; set; }
        public uint? Clan { get; set; }

        public Character()
        { }

        public Character(fb.protocol.db.raw.Character raw)
        {
            Id = raw.Id;
            Name = raw.Name;
            LastLogin = raw.LastLogin;
            Admin = raw.Admin;
            Look = raw.Look;
            Color = raw.Color;
            Sex = raw.Sex;
            Nation = raw.Nation;
            Creature = raw.Creature != null ? (ushort?)raw.Creature.Value.Value : null;
            Map = raw.Map;
            Position = new Position(raw.Position.Value);
            Direction = raw.Direction;
            State = raw.State;
            ClassType = raw.ClassType;
            Promotion = raw.Promotion;
            Exp = raw.Exp;
            Money = raw.Money;
            DepositedMoney = raw.DepositedMoney;
            Disguise = raw.Disguise != null ? (ushort?)raw.Disguise.Value.Value : null;
            Hp = raw.Hp;
            BaseHp = raw.BaseHp;
            AdditionalHp = raw.AdditionalHp;
            Mp = raw.Mp;
            BaseMp = raw.BaseMp;
            AdditionalMp = raw.AdditionalMp;
            WeaponColor = raw.WeaponColor != null ? (byte?)raw.WeaponColor.Value.Value : null;
            HelmetColor = raw.HelmetColor != null ? (byte?)raw.HelmetColor.Value.Value : null;
            ArmorColor = raw.ArmorColor != null ? (byte?)raw.ArmorColor.Value.Value : null;
            ShieldColor = raw.ShieldColor != null ? (byte?)raw.ShieldColor.Value.Value : null;
            RingLeftColor = raw.RingLeftColor != null ? (byte?)raw.RingLeftColor.Value.Value : null;
            RingRightColor = raw.RingRightColor != null ? (byte?)raw.RingRightColor.Value.Value : null;
            AuxTopColor = raw.AuxTopColor != null ? (byte?)raw.AuxTopColor.Value.Value : null;
            AuxBotColor = raw.AuxBotColor != null ? (byte?)raw.AuxBotColor.Value.Value : null;
            Clan = raw.Clan != null ? (uint?)raw.Clan.Value.Value : null;
        }

        public Character(byte[] bytes) : this(fb.protocol.db.raw.Character.GetRootAsCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.raw.Character> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.raw.Character.CreateCharacter(builder,
                Id,
                builder.CreateString(Name),
                builder.CreateString(LastLogin),
                Admin,
                Look,
                Color,
                Sex,
                Nation,
                Creature != null ? nullable_ushort.Createnullable_ushort(builder, Creature.Value) : default,
                Map,
                Position.Build(builder),
                Direction,
                State,
                ClassType,
                Promotion,
                Exp,
                Money,
                DepositedMoney,
                Disguise != null ? nullable_ushort.Createnullable_ushort(builder, Disguise.Value) : default,
                Hp,
                BaseHp,
                AdditionalHp,
                Mp,
                BaseMp,
                AdditionalMp,
                WeaponColor != null ? nullable_ubyte.Createnullable_ubyte(builder, WeaponColor.Value) : default,
                HelmetColor != null ? nullable_ubyte.Createnullable_ubyte(builder, HelmetColor.Value) : default,
                ArmorColor != null ? nullable_ubyte.Createnullable_ubyte(builder, ArmorColor.Value) : default,
                ShieldColor != null ? nullable_ubyte.Createnullable_ubyte(builder, ShieldColor.Value) : default,
                RingLeftColor != null ? nullable_ubyte.Createnullable_ubyte(builder, RingLeftColor.Value) : default,
                RingRightColor != null ? nullable_ubyte.Createnullable_ubyte(builder, RingRightColor.Value) : default,
                AuxTopColor != null ? nullable_ubyte.Createnullable_ubyte(builder, AuxTopColor.Value) : default,
                AuxBotColor != null ? nullable_ubyte.Createnullable_ubyte(builder, AuxBotColor.Value) : default,
                Clan != null ? nullable_uint.Createnullable_uint(builder, Clan.Value) : default);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Character Deserialize(byte[] bytes)
        {
            return new Character(bytes);
        }
    }
    public class Item : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Item;
        public uint User { get; set; }
        public short Index { get; set; }
        public short Parts { get; set; }
        public short Deposited { get; set; }
        public uint Model { get; set; }
        public ushort Count { get; set; }
        public uint? Durability { get; set; }
        public string CustomName { get; set; }

        public Item()
        { }

        public Item(fb.protocol.db.raw.Item raw)
        {
            User = raw.User;
            Index = raw.Index;
            Parts = raw.Parts;
            Deposited = raw.Deposited;
            Model = raw.Model;
            Count = raw.Count;
            Durability = raw.Durability != null ? (uint?)raw.Durability.Value.Value : null;
            CustomName = raw.CustomName;
        }

        public Item(byte[] bytes) : this(fb.protocol.db.raw.Item.GetRootAsItem(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.raw.Item> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.raw.Item.CreateItem(builder,
                User,
                Index,
                Parts,
                Deposited,
                Model,
                Count,
                Durability != null ? nullable_uint.Createnullable_uint(builder, Durability.Value) : default,
                CustomName != null ? builder.CreateString(CustomName) : default);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Item Deserialize(byte[] bytes)
        {
            return new Item(bytes);
        }
    }
    public class Spell : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Spell;
        public uint User { get; set; }
        public byte Slot { get; set; }
        public uint Id { get; set; }

        public Spell()
        { }

        public Spell(fb.protocol.db.raw.Spell raw)
        {
            User = raw.User;
            Slot = raw.Slot;
            Id = raw.Id;
        }

        public Spell(byte[] bytes) : this(fb.protocol.db.raw.Spell.GetRootAsSpell(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.raw.Spell> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.raw.Spell.CreateSpell(builder,
                User,
                Slot,
                Id);
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Spell Deserialize(byte[] bytes)
        {
            return new Spell(bytes);
        }
    }
    public class ArticleSummary : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ArticleSummary;
        public uint Id { get; set; }
        public uint User { get; set; }
        public string UserName { get; set; }
        public string Title { get; set; }
        public string CreatedDate { get; set; }

        public ArticleSummary()
        { }

        public ArticleSummary(fb.protocol.db.raw.ArticleSummary raw)
        {
            Id = raw.Id;
            User = raw.User;
            UserName = raw.UserName;
            Title = raw.Title;
            CreatedDate = raw.CreatedDate;
        }

        public ArticleSummary(byte[] bytes) : this(fb.protocol.db.raw.ArticleSummary.GetRootAsArticleSummary(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.raw.ArticleSummary> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.raw.ArticleSummary.CreateArticleSummary(builder,
                Id,
                User,
                builder.CreateString(UserName),
                builder.CreateString(Title),
                builder.CreateString(CreatedDate));
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ArticleSummary Deserialize(byte[] bytes)
        {
            return new ArticleSummary(bytes);
        }
    }
    public class Article : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Article;
        public uint Id { get; set; }
        public uint User { get; set; }
        public string UserName { get; set; }
        public string Title { get; set; }
        public string Contents { get; set; }
        public string CreatedDate { get; set; }

        public Article()
        { }

        public Article(fb.protocol.db.raw.Article raw)
        {
            Id = raw.Id;
            User = raw.User;
            UserName = raw.UserName;
            Title = raw.Title;
            Contents = raw.Contents;
            CreatedDate = raw.CreatedDate;
        }

        public Article(byte[] bytes) : this(fb.protocol.db.raw.Article.GetRootAsArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.raw.Article> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.raw.Article.CreateArticle(builder,
                Id,
                User,
                builder.CreateString(UserName),
                builder.CreateString(Title),
                builder.CreateString(Contents),
                builder.CreateString(CreatedDate));
        }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = Build(builder);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Article Deserialize(byte[] bytes)
        {
            return new Article(bytes);
        }
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Position => typeof(fb.protocol.db.Position),
                FlatBufferProtocolType.Character => typeof(fb.protocol.db.Character),
                FlatBufferProtocolType.Item => typeof(fb.protocol.db.Item),
                FlatBufferProtocolType.Spell => typeof(fb.protocol.db.Spell),
                FlatBufferProtocolType.ArticleSummary => typeof(fb.protocol.db.ArticleSummary),
                FlatBufferProtocolType.Article => typeof(fb.protocol.db.Article),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.db.request
{

    public class Account : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Account;
        public uint Uid { get; set; }

        public Account()
        { }

        public Account(fb.protocol.db.request.raw.Account raw)
        {
            Uid = raw.Uid;
        }

        public Account(byte[] bytes) : this(fb.protocol.db.request.raw.Account.GetRootAsAccount(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.Account> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.Account.CreateAccount(builder,
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
        public string Before { get; set; }
        public string After { get; set; }
        public uint Birthday { get; set; }

        public ChangePw()
        { }

        public ChangePw(fb.protocol.db.request.raw.ChangePw raw)
        {
            Uid = raw.Uid;
            Before = raw.Before;
            After = raw.After;
            Birthday = raw.Birthday;
        }

        public ChangePw(byte[] bytes) : this(fb.protocol.db.request.raw.ChangePw.GetRootAsChangePw(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.ChangePw> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.ChangePw.CreateChangePw(builder,
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
        public string Name { get; set; }
        public string Pw { get; set; }
        public uint Hp { get; set; }
        public uint Mp { get; set; }
        public ushort Map { get; set; }
        public ushort X { get; set; }
        public ushort Y { get; set; }
        public bool Admin { get; set; }

        public InitCharacter()
        { }

        public InitCharacter(fb.protocol.db.request.raw.InitCharacter raw)
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

        public InitCharacter(byte[] bytes) : this(fb.protocol.db.request.raw.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.InitCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.InitCharacter.CreateInitCharacter(builder,
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

        public Login(fb.protocol.db.request.raw.Login raw)
        {
            Uid = raw.Uid;
        }

        public Login(byte[] bytes) : this(fb.protocol.db.request.raw.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.Login> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.Login.CreateLogin(builder,
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

        public MakeCharacter(fb.protocol.db.request.raw.MakeCharacter raw)
        {
            Uid = raw.Uid;
            Hair = raw.Hair;
            Sex = raw.Sex;
            Nation = raw.Nation;
            Creature = raw.Creature;
        }

        public MakeCharacter(byte[] bytes) : this(fb.protocol.db.request.raw.MakeCharacter.GetRootAsMakeCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.MakeCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.MakeCharacter.CreateMakeCharacter(builder,
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
        public string Name { get; set; }

        public ReserveName()
        { }

        public ReserveName(fb.protocol.db.request.raw.ReserveName raw)
        {
            Name = raw.Name;
        }

        public ReserveName(byte[] bytes) : this(fb.protocol.db.request.raw.ReserveName.GetRootAsReserveName(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.ReserveName> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.ReserveName.CreateReserveName(builder,
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

        public Save(fb.protocol.db.request.raw.Save raw)
        {
            Character = new Character(raw.Character.Value);
            Items = Enumerable.Range(0, raw.ItemsLength).Select(i => raw.Items(i)).Select(x => new Item(x.Value)).ToList();
            Spells = Enumerable.Range(0, raw.SpellsLength).Select(i => raw.Spells(i)).Select(x => new Spell(x.Value)).ToList();
        }

        public Save(byte[] bytes) : this(fb.protocol.db.request.raw.Save.GetRootAsSave(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.Save> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.Save.CreateSave(builder,
                Character.Build(builder),
                fb.protocol.db.request.raw.Save.CreateItemsVector(builder, Items.Select(x => x.Build(builder)).ToArray()),
                fb.protocol.db.request.raw.Save.CreateSpellsVector(builder, Spells.Select(x => x.Build(builder)).ToArray()));
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
    public class GetArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetArticle;
        public uint Section { get; set; }
        public uint Article { get; set; }

        public GetArticle()
        { }

        public GetArticle(fb.protocol.db.request.raw.GetArticle raw)
        {
            Section = raw.Section;
            Article = raw.Article;
        }

        public GetArticle(byte[] bytes) : this(fb.protocol.db.request.raw.GetArticle.GetRootAsGetArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.GetArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.GetArticle.CreateGetArticle(builder,
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

        public GetArticleList(fb.protocol.db.request.raw.GetArticleList raw)
        {
            Section = raw.Section;
            Position = raw.Position;
        }

        public GetArticleList(byte[] bytes) : this(fb.protocol.db.request.raw.GetArticleList.GetRootAsGetArticleList(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.GetArticleList> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.GetArticleList.CreateGetArticleList(builder,
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
        public string Title { get; set; }
        public string Contents { get; set; }

        public WriteArticle()
        { }

        public WriteArticle(fb.protocol.db.request.raw.WriteArticle raw)
        {
            Section = raw.Section;
            User = raw.User;
            Title = raw.Title;
            Contents = raw.Contents;
        }

        public WriteArticle(byte[] bytes) : this(fb.protocol.db.request.raw.WriteArticle.GetRootAsWriteArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.WriteArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.WriteArticle.CreateWriteArticle(builder,
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
    public class DeleteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.DeleteArticle;
        public uint Id { get; set; }
        public uint User { get; set; }

        public DeleteArticle()
        { }

        public DeleteArticle(fb.protocol.db.request.raw.DeleteArticle raw)
        {
            Id = raw.Id;
            User = raw.User;
        }

        public DeleteArticle(byte[] bytes) : this(fb.protocol.db.request.raw.DeleteArticle.GetRootAsDeleteArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.request.raw.DeleteArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.request.raw.DeleteArticle.CreateDeleteArticle(builder,
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

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Account => typeof(fb.protocol.db.request.Account),
                FlatBufferProtocolType.ChangePw => typeof(fb.protocol.db.request.ChangePw),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol.db.request.InitCharacter),
                FlatBufferProtocolType.Login => typeof(fb.protocol.db.request.Login),
                FlatBufferProtocolType.MakeCharacter => typeof(fb.protocol.db.request.MakeCharacter),
                FlatBufferProtocolType.ReserveName => typeof(fb.protocol.db.request.ReserveName),
                FlatBufferProtocolType.Save => typeof(fb.protocol.db.request.Save),
                FlatBufferProtocolType.GetArticle => typeof(fb.protocol.db.request.GetArticle),
                FlatBufferProtocolType.GetArticleList => typeof(fb.protocol.db.request.GetArticleList),
                FlatBufferProtocolType.WriteArticle => typeof(fb.protocol.db.request.WriteArticle),
                FlatBufferProtocolType.DeleteArticle => typeof(fb.protocol.db.request.DeleteArticle),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.db.response
{

    public class Account : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Account;
        public string Pw { get; set; }
        public uint Map { get; set; }
        public bool Success { get; set; }

        public Account()
        { }

        public Account(fb.protocol.db.response.raw.Account raw)
        {
            Pw = raw.Pw;
            Map = raw.Map;
            Success = raw.Success;
        }

        public Account(byte[] bytes) : this(fb.protocol.db.response.raw.Account.GetRootAsAccount(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.Account> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.Account.CreateAccount(builder,
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

        public DeleteArticle(fb.protocol.db.response.raw.DeleteArticle raw)
        {
            Result = raw.Result;
        }

        public DeleteArticle(byte[] bytes) : this(fb.protocol.db.response.raw.DeleteArticle.GetRootAsDeleteArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.DeleteArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.DeleteArticle.CreateDeleteArticle(builder,
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

        public GetArticle(fb.protocol.db.response.raw.GetArticle raw)
        {
            Article = new Article(raw.Article.Value);
            Next = raw.Next;
            Success = raw.Success;
        }

        public GetArticle(byte[] bytes) : this(fb.protocol.db.response.raw.GetArticle.GetRootAsGetArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.GetArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.GetArticle.CreateGetArticle(builder,
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

        public GetArticleList(fb.protocol.db.response.raw.GetArticleList raw)
        {
            SummaryList = Enumerable.Range(0, raw.SummaryListLength).Select(i => raw.SummaryList(i)).Select(x => new ArticleSummary(x.Value)).ToList();
        }

        public GetArticleList(byte[] bytes) : this(fb.protocol.db.response.raw.GetArticleList.GetRootAsGetArticleList(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.GetArticleList> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.GetArticleList.CreateGetArticleList(builder,
                fb.protocol.db.response.raw.GetArticleList.CreateSummaryListVector(builder, SummaryList.Select(x => x.Build(builder)).ToArray()));
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

        public WriteArticle(fb.protocol.db.response.raw.WriteArticle raw)
        {
            Success = raw.Success;
        }

        public WriteArticle(byte[] bytes) : this(fb.protocol.db.response.raw.WriteArticle.GetRootAsWriteArticle(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.WriteArticle> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.WriteArticle.CreateWriteArticle(builder,
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

        public ChangePw(fb.protocol.db.response.raw.ChangePw raw)
        {
            Result = raw.Result;
        }

        public ChangePw(byte[] bytes) : this(fb.protocol.db.response.raw.ChangePw.GetRootAsChangePw(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.ChangePw> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.ChangePw.CreateChangePw(builder,
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

        public GetUid(fb.protocol.db.response.raw.GetUid raw)
        {
            Uid = raw.Uid;
            Success = raw.Success;
        }

        public GetUid(byte[] bytes) : this(fb.protocol.db.response.raw.GetUid.GetRootAsGetUid(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.GetUid> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.GetUid.CreateGetUid(builder,
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

        public InitCharacter(fb.protocol.db.response.raw.InitCharacter raw)
        {
            Success = raw.Success;
        }

        public InitCharacter(byte[] bytes) : this(fb.protocol.db.response.raw.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.InitCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.InitCharacter.CreateInitCharacter(builder,
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

        public Login(fb.protocol.db.response.raw.Login raw)
        {
            Character = new Character(raw.Character.Value);
            Items = Enumerable.Range(0, raw.ItemsLength).Select(i => raw.Items(i)).Select(x => new Item(x.Value)).ToList();
            Spells = Enumerable.Range(0, raw.SpellsLength).Select(i => raw.Spells(i)).Select(x => new Spell(x.Value)).ToList();
        }

        public Login(byte[] bytes) : this(fb.protocol.db.response.raw.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.Login> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.Login.CreateLogin(builder,
                Character.Build(builder),
                fb.protocol.db.response.raw.Login.CreateItemsVector(builder, Items.Select(x => x.Build(builder)).ToArray()),
                fb.protocol.db.response.raw.Login.CreateSpellsVector(builder, Spells.Select(x => x.Build(builder)).ToArray()));
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

        public MakeCharacter(fb.protocol.db.response.raw.MakeCharacter raw)
        {
            Success = raw.Success;
        }

        public MakeCharacter(byte[] bytes) : this(fb.protocol.db.response.raw.MakeCharacter.GetRootAsMakeCharacter(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.MakeCharacter> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.MakeCharacter.CreateMakeCharacter(builder,
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

        public ReserveName(fb.protocol.db.response.raw.ReserveName raw)
        {
            Uid = raw.Uid;
            Success = raw.Success;
        }

        public ReserveName(byte[] bytes) : this(fb.protocol.db.response.raw.ReserveName.GetRootAsReserveName(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.ReserveName> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.ReserveName.CreateReserveName(builder,
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

        public Save(fb.protocol.db.response.raw.Save raw)
        {
            Success = raw.Success;
        }

        public Save(byte[] bytes) : this(fb.protocol.db.response.raw.Save.GetRootAsSave(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol.db.response.raw.Save> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.response.raw.Save.CreateSave(builder,
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

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Account => typeof(fb.protocol.db.response.Account),
                FlatBufferProtocolType.DeleteArticle => typeof(fb.protocol.db.response.DeleteArticle),
                FlatBufferProtocolType.GetArticle => typeof(fb.protocol.db.response.GetArticle),
                FlatBufferProtocolType.GetArticleList => typeof(fb.protocol.db.response.GetArticleList),
                FlatBufferProtocolType.WriteArticle => typeof(fb.protocol.db.response.WriteArticle),
                FlatBufferProtocolType.ChangePw => typeof(fb.protocol.db.response.ChangePw),
                FlatBufferProtocolType.GetUid => typeof(fb.protocol.db.response.GetUid),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol.db.response.InitCharacter),
                FlatBufferProtocolType.Login => typeof(fb.protocol.db.response.Login),
                FlatBufferProtocolType.MakeCharacter => typeof(fb.protocol.db.response.MakeCharacter),
                FlatBufferProtocolType.ReserveName => typeof(fb.protocol.db.response.ReserveName),
                FlatBufferProtocolType.Save => typeof(fb.protocol.db.response.Save),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol._internal
{
    public enum Service
    {
        Gateway,
        Login,
        Game,
    }
    public enum TransferResult
    {
        Success,
        Failed,
        LoggedIn,
    }


    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol._internal.request
{

    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public uint Uid { get; set; }
        public string Name { get; set; }
        public ushort Map { get; set; }

        public Login()
        { }

        public Login(fb.protocol._internal.request.raw.Login raw)
        {
            Uid = raw.Uid;
            Name = raw.Name;
            Map = raw.Map;
        }

        public Login(byte[] bytes) : this(fb.protocol._internal.request.raw.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.request.raw.Login> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                Uid,
                builder.CreateString(Name),
                Map);
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
        public string Name { get; set; }

        public Logout()
        { }

        public Logout(fb.protocol._internal.request.raw.Logout raw)
        {
            Name = raw.Name;
        }

        public Logout(byte[] bytes) : this(fb.protocol._internal.request.raw.Logout.GetRootAsLogout(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.request.raw.Logout> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.request.raw.Logout.CreateLogout(builder,
                builder.CreateString(Name));
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
        public int ProtocolType => (int)FlatBufferProtocolType.Ping;
        public byte Id { get; set; }
        public string Name { get; set; }
        public fb.protocol._internal.Service Service { get; set; }
        public string Ip { get; set; }
        public ushort Port { get; set; }

        public Ping()
        { }

        public Ping(fb.protocol._internal.request.raw.Ping raw)
        {
            Id = raw.Id;
            Name = raw.Name;
            Service = (fb.protocol._internal.Service)raw.Service;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Ping(byte[] bytes) : this(fb.protocol._internal.request.raw.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.request.raw.Ping> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.request.raw.Ping.CreatePing(builder,
                Id,
                builder.CreateString(Name),
                (fb.protocol._internal.raw.Service)Service,
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
        public int ProtocolType => (int)FlatBufferProtocolType.Transfer;
        public fb.protocol._internal.Service Service { get; set; }
        public byte Id { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol._internal.request.raw.Transfer raw)
        {
            Service = (fb.protocol._internal.Service)raw.Service;
            Id = raw.Id;
        }

        public Transfer(byte[] bytes) : this(fb.protocol._internal.request.raw.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.request.raw.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.request.raw.Transfer.CreateTransfer(builder,
                (fb.protocol._internal.raw.Service)Service,
                Id);
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
        public string From { get; set; }
        public string To { get; set; }
        public string Message { get; set; }

        public Whisper()
        { }

        public Whisper(fb.protocol._internal.request.raw.Whisper raw)
        {
            From = raw.From;
            To = raw.To;
            Message = raw.Message;
        }

        public Whisper(byte[] bytes) : this(fb.protocol._internal.request.raw.Whisper.GetRootAsWhisper(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.request.raw.Whisper> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.request.raw.Whisper.CreateWhisper(builder,
                builder.CreateString(From),
                builder.CreateString(To),
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
    public class KickOut : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.KickOut;
        public uint Uid { get; set; }

        public KickOut()
        { }

        public KickOut(fb.protocol._internal.request.raw.KickOut raw)
        {
            Uid = raw.Uid;
        }

        public KickOut(byte[] bytes) : this(fb.protocol._internal.request.raw.KickOut.GetRootAsKickOut(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.request.raw.KickOut> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.request.raw.KickOut.CreateKickOut(builder,
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

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Login => typeof(fb.protocol._internal.request.Login),
                FlatBufferProtocolType.Logout => typeof(fb.protocol._internal.request.Logout),
                FlatBufferProtocolType.Ping => typeof(fb.protocol._internal.request.Ping),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol._internal.request.Transfer),
                FlatBufferProtocolType.Whisper => typeof(fb.protocol._internal.request.Whisper),
                FlatBufferProtocolType.KickOut => typeof(fb.protocol._internal.request.KickOut),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol._internal.response
{

    public class KickOut : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.KickOut;
        public uint Uid { get; set; }

        public KickOut()
        { }

        public KickOut(fb.protocol._internal.response.raw.KickOut raw)
        {
            Uid = raw.Uid;
        }

        public KickOut(byte[] bytes) : this(fb.protocol._internal.response.raw.KickOut.GetRootAsKickOut(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.response.raw.KickOut> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
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
        public string Ip { get; set; }
        public ushort Port { get; set; }

        public Login()
        { }

        public Login(fb.protocol._internal.response.raw.Login raw)
        {
            Success = raw.Success;
            Logon = raw.Logon;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Login(byte[] bytes) : this(fb.protocol._internal.response.raw.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.response.raw.Login> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
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
        public bool Success { get; set; }

        public Logout()
        { }

        public Logout(fb.protocol._internal.response.raw.Logout raw)
        {
            Success = raw.Success;
        }

        public Logout(byte[] bytes) : this(fb.protocol._internal.response.raw.Logout.GetRootAsLogout(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.response.raw.Logout> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.response.raw.Logout.CreateLogout(builder,
                Success);
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

        public Pong(fb.protocol._internal.response.raw.Pong raw)
        {
        }

        public Pong(byte[] bytes) : this(fb.protocol._internal.response.raw.Pong.GetRootAsPong(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.response.raw.Pong> Build(FlatBufferBuilder builder)
        {
            fb.protocol._internal.response.raw.Pong.StartPong(builder);
            return fb.protocol._internal.response.raw.Pong.EndPong(builder);
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
        public fb.protocol._internal.TransferResult Code { get; set; }
        public string Ip { get; set; }
        public ushort Port { get; set; }

        public Transfer()
        { }

        public Transfer(fb.protocol._internal.response.raw.Transfer raw)
        {
            Code = (fb.protocol._internal.TransferResult)raw.Code;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Transfer(byte[] bytes) : this(fb.protocol._internal.response.raw.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.response.raw.Transfer> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.response.raw.Transfer.CreateTransfer(builder,
                (fb.protocol._internal.raw.TransferResult)Code,
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
        public string From { get; set; }
        public uint To { get; set; }
        public string Message { get; set; }

        public Whisper()
        { }

        public Whisper(fb.protocol._internal.response.raw.Whisper raw)
        {
            Success = raw.Success;
            From = raw.From;
            To = raw.To;
            Message = raw.Message;
        }

        public Whisper(byte[] bytes) : this(fb.protocol._internal.response.raw.Whisper.GetRootAsWhisper(new ByteBuffer(bytes)))
        { }

        public Offset<fb.protocol._internal.response.raw.Whisper> Build(FlatBufferBuilder builder)
        {
            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
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

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.KickOut => typeof(fb.protocol._internal.response.KickOut),
                FlatBufferProtocolType.Login => typeof(fb.protocol._internal.response.Login),
                FlatBufferProtocolType.Logout => typeof(fb.protocol._internal.response.Logout),
                FlatBufferProtocolType.Pong => typeof(fb.protocol._internal.response.Pong),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol._internal.response.Transfer),
                FlatBufferProtocolType.Whisper => typeof(fb.protocol._internal.response.Whisper),
                _ => throw new ArgumentException(),
            };
        }
    }
}
