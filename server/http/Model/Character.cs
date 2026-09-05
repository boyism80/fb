using StackExchange.Redis;

namespace Http.Model
{
    public class CharacterKey : BaseModel, IRedisValueKey
    {
        public required uint Id { get; set; }
        public uint? GetHash() => Id;

        public RedisKey GetRedisKey() => $"fb:cache:user:{Id}";
    }

    public class Character : CharacterKey, IModel
    {
        public uint World { get; set; }
        public string Name { get; set; }
        public string Pw { get; set; }
        public uint? Birth { get; set; }
        public Fb.Model.EnumValue.Role Role { get; set; }
        public ushort Hair { get; set; }
        public byte Face { get; set; }
        public ushort RidableId { get; set; } = 1;
        public byte Color { get; set; }
        public byte Gender { get; set; }
        public byte Nation { get; set; }
        public byte? DivineBeast { get; set; }
        public uint Map { get; set; }
        public ushort PositionX { get; set; }
        public ushort PositionY { get; set; }
        public byte Direction { get; set; }
        public byte State { get; set; }
        public byte Class { get; set; }
        public byte Promotion { get; set; }
        public byte Level { get; set; }
        public ulong Exp { get; set; }
        public ulong Money { get; set; }
        public ulong DepositedMoney { get; set; }
        public Mimicry Mimicry { get; set; }
        public ulong Hp { get; set; }
        public ulong BaseHp { get; set; }
        public ulong AdditionalHp { get; set; }
        public ulong Mp { get; set; }
        public ulong BaseMp { get; set; }
        public ulong AdditionalMp { get; set; }
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
        public bool SuperHide { get; set; }
        public byte Speed { get; set; }
        public short Reputation { get; set; }
        public ushort Evaluation { get; set; }
        public DateTime? FirstLoginDate { get; set; }
    }
}
