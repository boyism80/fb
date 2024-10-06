using Google.FlatBuffers;

namespace fb.protocol.db
{
    public class nullopt_ushort
    {
        public ushort Value { get; set; }
        public bool Empty { get; set; }

        public nullopt_ushort()
        { }

        public nullopt_ushort(fb.protocol.db.origin.nullopt_ushort raw)
        {
            Value = raw.Value;
            Empty = raw.Empty;
        }


        public Offset<fb.protocol.db.origin.nullopt_ushort> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.nullopt_ushort.Createnullopt_ushort(builder,
                Value,
                Empty);
        }
    }
    public class nullopt_ubyte
    {
        public byte Value { get; set; }
        public bool Empty { get; set; }

        public nullopt_ubyte()
        { }

        public nullopt_ubyte(fb.protocol.db.origin.nullopt_ubyte raw)
        {
            Value = raw.Value;
            Empty = raw.Empty;
        }


        public Offset<fb.protocol.db.origin.nullopt_ubyte> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.nullopt_ubyte.Createnullopt_ubyte(builder,
                Value,
                Empty);
        }
    }
    public class nullopt_uint
    {
        public uint Value { get; set; }
        public bool Empty { get; set; }

        public nullopt_uint()
        { }

        public nullopt_uint(fb.protocol.db.origin.nullopt_uint raw)
        {
            Value = raw.Value;
            Empty = raw.Empty;
        }


        public Offset<fb.protocol.db.origin.nullopt_uint> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.nullopt_uint.Createnullopt_uint(builder,
                Value,
                Empty);
        }
    }
    public class Position
    {
        public ushort X { get; set; }
        public ushort Y { get; set; }

        public Position()
        { }

        public Position(fb.protocol.db.origin.Position raw)
        {
            X = raw.X;
            Y = raw.Y;
        }


        public Offset<fb.protocol.db.origin.Position> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.Position.CreatePosition(builder,
                X,
                Y);
        }
    }
    public class Character
    {
        public uint Id { get; set; }
        public string Name { get; set; } = string.Empty;
        public string LastLogin { get; set; } = string.Empty;
        public bool Admin { get; set; }
        public ushort Look { get; set; }
        public ushort Color { get; set; }
        public ushort Sex { get; set; }
        public ushort Nation { get; set; }
        public nullopt_ushort Creature { get; set; }
        public uint Map { get; set; }
        public Position Position { get; set; }
        public byte Direction { get; set; }
        public byte State { get; set; }
        public byte ClassType { get; set; }
        public byte Promotion { get; set; }
        public uint Exp { get; set; }
        public uint Money { get; set; }
        public uint DepositedMoney { get; set; }
        public nullopt_ushort Disguise { get; set; }
        public uint Hp { get; set; }
        public uint BaseHp { get; set; }
        public uint AdditionalHp { get; set; }
        public uint Mp { get; set; }
        public uint BaseMp { get; set; }
        public uint AdditionalMp { get; set; }
        public nullopt_ubyte WeaponColor { get; set; }
        public nullopt_ubyte HelmetColor { get; set; }
        public nullopt_ubyte ArmorColor { get; set; }
        public nullopt_ubyte ShieldColor { get; set; }
        public nullopt_ubyte RingLeftColor { get; set; }
        public nullopt_ubyte RingRightColor { get; set; }
        public nullopt_ubyte AuxTopColor { get; set; }
        public nullopt_ubyte AuxBotColor { get; set; }
        public nullopt_uint Clan { get; set; }

        public Character()
        { }

        public Character(fb.protocol.db.origin.Character raw)
        {
            Id = raw.Id;
            Name = raw.Name;
            LastLogin = raw.LastLogin;
            Admin = raw.Admin;
            Look = raw.Look;
            Color = raw.Color;
            Sex = raw.Sex;
            Nation = raw.Nation;
            Creature = new nullopt_ushort(raw.Creature.Value);
            Map = raw.Map;
            Position = new Position(raw.Position.Value);
            Direction = raw.Direction;
            State = raw.State;
            ClassType = raw.ClassType;
            Promotion = raw.Promotion;
            Exp = raw.Exp;
            Money = raw.Money;
            DepositedMoney = raw.DepositedMoney;
            Disguise = new nullopt_ushort(raw.Disguise.Value);
            Hp = raw.Hp;
            BaseHp = raw.BaseHp;
            AdditionalHp = raw.AdditionalHp;
            Mp = raw.Mp;
            BaseMp = raw.BaseMp;
            AdditionalMp = raw.AdditionalMp;
            WeaponColor = new nullopt_ubyte(raw.WeaponColor.Value);
            HelmetColor = new nullopt_ubyte(raw.HelmetColor.Value);
            ArmorColor = new nullopt_ubyte(raw.ArmorColor.Value);
            ShieldColor = new nullopt_ubyte(raw.ShieldColor.Value);
            RingLeftColor = new nullopt_ubyte(raw.RingLeftColor.Value);
            RingRightColor = new nullopt_ubyte(raw.RingRightColor.Value);
            AuxTopColor = new nullopt_ubyte(raw.AuxTopColor.Value);
            AuxBotColor = new nullopt_ubyte(raw.AuxBotColor.Value);
            Clan = new nullopt_uint(raw.Clan.Value);
        }


        public Offset<fb.protocol.db.origin.Character> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.Character.CreateCharacter(builder,
                Id,
                builder.CreateString(Name),
                builder.CreateString(LastLogin),
                Admin,
                Look,
                Color,
                Sex,
                Nation,
                Creature.Build(builder),
                Map,
                Position.Build(builder),
                Direction,
                State,
                ClassType,
                Promotion,
                Exp,
                Money,
                DepositedMoney,
                Disguise.Build(builder),
                Hp,
                BaseHp,
                AdditionalHp,
                Mp,
                BaseMp,
                AdditionalMp,
                WeaponColor.Build(builder),
                HelmetColor.Build(builder),
                ArmorColor.Build(builder),
                ShieldColor.Build(builder),
                RingLeftColor.Build(builder),
                RingRightColor.Build(builder),
                AuxTopColor.Build(builder),
                AuxBotColor.Build(builder),
                Clan.Build(builder));
        }
    }
    public class Item
    {
        public uint User { get; set; }
        public short Index { get; set; }
        public short Parts { get; set; }
        public short Deposited { get; set; }
        public uint Model { get; set; }
        public ushort Count { get; set; }
        public nullopt_uint Durability { get; set; }
        public string CustomName { get; set; } = string.Empty;

        public Item()
        { }

        public Item(fb.protocol.db.origin.Item raw)
        {
            User = raw.User;
            Index = raw.Index;
            Parts = raw.Parts;
            Deposited = raw.Deposited;
            Model = raw.Model;
            Count = raw.Count;
            Durability = new nullopt_uint(raw.Durability.Value);
            CustomName = raw.CustomName;
        }


        public Offset<fb.protocol.db.origin.Item> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.Item.CreateItem(builder,
                User,
                Index,
                Parts,
                Deposited,
                Model,
                Count,
                Durability.Build(builder),
                builder.CreateString(CustomName));
        }
    }
    public class Spell
    {
        public uint User { get; set; }
        public byte Slot { get; set; }
        public uint Id { get; set; }

        public Spell()
        { }

        public Spell(fb.protocol.db.origin.Spell raw)
        {
            User = raw.User;
            Slot = raw.Slot;
            Id = raw.Id;
        }


        public Offset<fb.protocol.db.origin.Spell> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.Spell.CreateSpell(builder,
                User,
                Slot,
                Id);
        }
    }
    public class ArticleSummary
    {
        public uint Id { get; set; }
        public uint User { get; set; }
        public string UserName { get; set; } = string.Empty;
        public string Title { get; set; } = string.Empty;
        public string CreatedDate { get; set; } = string.Empty;

        public ArticleSummary()
        { }

        public ArticleSummary(fb.protocol.db.origin.ArticleSummary raw)
        {
            Id = raw.Id;
            User = raw.User;
            UserName = raw.UserName;
            Title = raw.Title;
            CreatedDate = raw.CreatedDate;
        }


        public Offset<fb.protocol.db.origin.ArticleSummary> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.ArticleSummary.CreateArticleSummary(builder,
                Id,
                User,
                builder.CreateString(UserName),
                builder.CreateString(Title),
                builder.CreateString(CreatedDate));
        }
    }
    public class Article
    {
        public uint Id { get; set; }
        public uint User { get; set; }
        public string UserName { get; set; } = string.Empty;
        public string Title { get; set; } = string.Empty;
        public string Contents { get; set; } = string.Empty;
        public string CreatedDate { get; set; } = string.Empty;

        public Article()
        { }

        public Article(fb.protocol.db.origin.Article raw)
        {
            Id = raw.Id;
            User = raw.User;
            UserName = raw.UserName;
            Title = raw.Title;
            Contents = raw.Contents;
            CreatedDate = raw.CreatedDate;
        }


        public Offset<fb.protocol.db.origin.Article> Build(FlatBufferBuilder builder)
        {
            return fb.protocol.db.origin.Article.CreateArticle(builder,
                Id,
                User,
                builder.CreateString(UserName),
                builder.CreateString(Title),
                builder.CreateString(Contents),
                builder.CreateString(CreatedDate));
        }
    }
}
