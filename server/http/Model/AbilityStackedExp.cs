using Fb.Model.EnumValue;

namespace Fb.Model
{
    public static class AbilityStackedExp
    {
        private static readonly Dictionary<Class, Dictionary<byte, ulong>> Cache = new();

        public static void BuildCache()
        {
            Cache.Clear();

            var intra = new Dictionary<Class, Dictionary<byte, ulong>>();

            foreach (var (cls, levels) in Table.Ability)
            {
                var map = new Dictionary<byte, ulong>();
                ulong sum = 0;

                foreach (var level in levels.Keys.OrderBy(k => k))
                {
                    sum += levels[level].Exp;
                    map[level] = sum;
                }

                intra[cls] = map;
            }

            foreach (var (cls, classIntra) in intra)
            {
                if (cls == Class.None)
                {
                    Cache[cls] = classIntra;
                    continue;
                }

                ulong noneBase = 0;
                if (intra.TryGetValue(Class.None, out var noneIntra))
                {
                    var minLevel = classIntra.Keys.Min();
                    var prevLevel = (byte)(minLevel - 1);
                    if (prevLevel > 0 && noneIntra.TryGetValue(prevLevel, out var noneStacked))
                        noneBase = noneStacked;
                }

                Cache[cls] = classIntra.ToDictionary(
                    kvp => kvp.Key,
                    kvp => noneBase + kvp.Value);
            }
        }

        public static ulong GetStackedExp(Class cls, byte level)
        {
            if (Cache.TryGetValue(cls, out var levels) && levels.TryGetValue(level, out var stacked))
                return stacked;

            return 0;
        }

        public static bool TryGetStackedExp(Class cls, byte level, out ulong stacked)
        {
            stacked = 0;
            return Cache.TryGetValue(cls, out var levels) && levels.TryGetValue(level, out stacked);
        }
    }
}
