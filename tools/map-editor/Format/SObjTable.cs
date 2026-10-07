namespace MapEditor.Format
{
    public class SObj
    {
        /// <summary>
        /// Blocked directions: S=1, N=2, W=4, E=8. 0x0F blocks the cell on the server.
        /// </summary>
        public byte Collision { get; init; }

        /// <summary>
        /// TILEC frame per stacked cell; frames[k] is drawn k cells above the object's cell. 0 = empty.
        /// </summary>
        public ushort[] Frames { get; init; }
    }

    /// <summary>
    /// SOBJ.TBL: u32 count, u16 ?, then count x { i32 anim_id, u8 ?, u8 collision, u8 height, u16 frames[height] }.
    /// Map object id N is record N-1; 0 means no object.
    /// </summary>
    public class SObjTable
    {
        private readonly List<SObj> _records = new List<SObj>();

        public int Count => _records.Count;
        public int MaxHeight { get; private set; }

        public static SObjTable Read(byte[] bytes)
        {
            var table = new SObjTable();
            var count = BitConverter.ToInt32(bytes, 0);
            var position = 6;
            for (int i = 0; i < count; i++)
            {
                var collision = bytes[position + 5];
                var height = bytes[position + 6];
                var frames = new ushort[height];
                for (int k = 0; k < height; k++)
                    frames[k] = BitConverter.ToUInt16(bytes, position + 7 + k * 2);

                table._records.Add(new SObj { Collision = collision, Frames = frames });
                table.MaxHeight = Math.Max(table.MaxHeight, height);
                position += 7 + height * 2;
            }
            return table;
        }

        public SObj Find(int objectId)
        {
            if (objectId <= 0 || objectId > _records.Count)
                return null;

            return _records[objectId - 1];
        }
    }
}
