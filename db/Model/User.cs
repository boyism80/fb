namespace db.Model
{
    public class User
    {
        public uint Id { get; set; }
        public string Name { get; set; }
        public string Pw { get; set; }
        public DateTime Birth { get; set; }
        public DateTime LastLogin { get; set; }
        public bool Admin { get; set; }
        public ushort Look { get; set; }
        public ushort Color { get; set; }
        public ushort Sex { get; set; }
        public ushort Nation { get; set; }
        public ushort Creature { get; set; }
        public ushort Map { get; set; }
        public ushort PositionX { get; set; }
        public ushort PositionY { get; set; }
        public byte Direction { get; set; }
        public byte State { get; set; }
        public byte Class { get; set; }
        public byte Promotion { get; set; }
        public uint Exp { get; set; }
        public uint Money { get; set; }
        public uint DepositedMoney { get; set; }
        public ushort Disguise { get; set; }
        public uint Hp { get; set; }
        public uint BaseHp { get; set; }
        public uint AdditionalHp { get; set; }
        public uint Mp { get; set; }
        public uint BaseMp { get; set; }
        public uint AdditionalMp { get; set; }
        public byte WeaponColor { get; set; }
        public byte HelmetColor { get; set; }
        public byte ArmorColor { get; set; }
        public byte ShieldColor { get; set; }
        public byte RingLeftColor { get; set; }
        public byte RingRightColor { get; set; }
        public byte AuxTopColor { get; set; }
        public byte AuxBotColor { get; set; }
        public uint Clan { get; set; }
    }
}
