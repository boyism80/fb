using StackExchange.Redis;

namespace Http.Model
{
    public class CharacterKey : BaseModel, IRedisValueKey
    {
        public required uint Id { get; set; }
        public uint? GetHash() => Id;

        public RedisKey GetRedisKey() => $"cache:user:{Id}";
    }

    public class Character : CharacterKey, IModel
    {
        public string Name { get; set; }
        public string Pw { get; set; }
        public uint? Birth { get; set; }
        public Fb.Model.EnumValue.Role Role { get; set; }
        public ushort Look { get; set; }
        public byte Color { get; set; }
        public byte Gender { get; set; }
        public byte Nation { get; set; }
        public byte? Creature { get; set; }
        public uint Map { get; set; }
        public ushort PositionX { get; set; }
        public ushort PositionY { get; set; }
        public byte Direction { get; set; }
        public byte State { get; set; }
        public byte Class { get; set; }
        public byte Promotion { get; set; }
        public byte Level { get; set; }
        public uint Exp { get; set; }
        public uint Money { get; set; }
        public uint DepositedMoney { get; set; }
        public Mimicry Mimicry { get; set; }
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
        public List<Buff> Buffs { get; set; } = new List<Buff>();
        public string Title { get; set; } = string.Empty;
        public Dictionary<string, List<Fb.Model.Dsl>> PendingListings { get; set; } = new Dictionary<string, List<Fb.Model.Dsl>>();
        public bool SuperHide { get; set; }
        public DateTime? FirstLoginDate { get; set; }
    }
}
