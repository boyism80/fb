namespace Db.Model
{
    public class Item
    {
        public uint Owner { get; set; }
        public short Index { get; set; } = -1;
        public short Parts { get; set; } = 0;
        public short Deposited { get; set; } = -1;
        public uint Model { get; set; }
        public ushort Count { get; set; } = 1;
        public uint? Durability { get; set; }
        public string CustomName { get; set; }
    }
}
