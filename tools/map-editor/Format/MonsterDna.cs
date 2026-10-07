namespace MapEditor.Format
{
    public class MonsterRecord
    {
        public int BaseFrame;
        public int Palette;

        /// <summary>
        /// First frame of each animation relative to BaseFrame, -1 when the animation has no frames.
        /// 1..4 are the idle poses TOP, RIGHT, BOTTOM, LEFT.
        /// </summary>
        public int[] FirstFrames;
    }

    /// <summary>
    /// MONSTER.DNA (5.50 MON.DAT, 6.51 DATA/MON.DAT): u32 count, then per record
    /// { u32 base_frame; u8 anim_count; u8 shadow; u16 palette; anim_count x { u16 frame_count; frame_count x 9 bytes } }
    /// with each frame { u16 frame; u16 delay; u16 ?; u8 alpha; u8 ?; u8 ? }.
    /// </summary>
    public static class MonsterDna
    {
        public static List<MonsterRecord> Read(byte[] bytes)
        {
            var count = BitConverter.ToInt32(bytes, 0);
            var records = new List<MonsterRecord>(count);
            var position = 4;
            for (int i = 0; i < count; i++)
            {
                var record = new MonsterRecord
                {
                    BaseFrame = BitConverter.ToInt32(bytes, position),
                    Palette = BitConverter.ToUInt16(bytes, position + 6),
                    FirstFrames = new int[bytes[position + 4]],
                };
                position += 8;
                for (int a = 0; a < record.FirstFrames.Length; a++)
                {
                    var frames = BitConverter.ToUInt16(bytes, position);
                    position += 2;
                    record.FirstFrames[a] = frames > 0 ? BitConverter.ToUInt16(bytes, position) : -1;
                    position += frames * 9;
                }
                records.Add(record);
            }
            return records;
        }
    }
}
