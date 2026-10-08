using MapEditor.Format;

namespace MapEditor.Edit
{
    /// <summary>
    /// Transition tiles between two fill tiles A and B, learned from the server maps with the corner model: each tile
    /// has four corners that are A or B, and neighbouring tiles agree on the corners they share.
    /// Pattern bits: 1 NW, 2 NE, 4 SW, 8 SE; a set bit is a B corner. B cells carry the edges (a B cell whose north
    /// neighbour is A gets pattern SW|SE), so a painted area keeps its size.
    /// </summary>
    public class TerrainSet
    {
        public int A { get; private init; }
        public int B { get; private init; }

        /// <summary>
        /// Tiles per pattern with their corpus count and how sure the corner estimate is (0..1).
        /// </summary>
        public List<(int Tile, long Count, double Confidence)>[] Patterns { get; } = Enumerable.Range(0, 16).Select(_ => new List<(int, long, double)>()).ToArray();

        public Dictionary<int, int> PatternOf { get; } = new Dictionary<int, int>();

        private const double MinConfidence = 0.6;

        /// <summary>
        /// For corner NW, NE, SW, SE: the three neighbours sharing it and which of their corners it is.
        /// </summary>
        private static readonly (int Dx, int Dy, int Corner)[][] Shared =
        {
            new[] { (-1, 0, 1), (0, -1, 2), (-1, -1, 3) },
            new[] { (1, 0, 0), (0, -1, 3), (1, -1, 2) },
            new[] { (-1, 0, 3), (0, 1, 0), (-1, 1, 1) },
            new[] { (1, 0, 2), (0, 1, 1), (1, 1, 0) },
        };

        public static TerrainSet Learn(IEnumerable<ServerMap> corpus, int a, int b)
        {
            var maps = corpus.Where(m => Array.IndexOf(m.Tiles, (ushort)a) >= 0 && Array.IndexOf(m.Tiles, (ushort)b) >= 0).ToList();
            if (maps.Count == 0)
                throw new ArgumentException($"no map uses both tile {a} and tile {b}");

            // Candidates: tiles that often touch both A and B.
            var occurrences = new Dictionary<int, long>();
            var touchA = new Dictionary<int, long>();
            var touchB = new Dictionary<int, long>();
            long countA = 0, countB = 0;
            foreach (var map in maps)
            {
                for (int y = 0; y < map.Height; y++)
                {
                    for (int x = 0; x < map.Width; x++)
                    {
                        int t = map.Tiles[y * map.Width + x];
                        if (t == a)
                        {
                            countA++;
                            continue;
                        }
                        else if (t == b)
                        {
                            countB++;
                            continue;
                        }

                        bool hasA = false, hasB = false;
                        for (int dy = -1; dy <= 1; dy++)
                        {
                            for (int dx = -1; dx <= 1; dx++)
                            {
                                if (map.Contains(x + dx, y + dy) == false)
                                    continue;
                                int n = map.Tiles[(y + dy) * map.Width + x + dx];
                                hasA |= n == a;
                                hasB |= n == b;
                            }
                        }
                        if (hasA == false && hasB == false)
                            continue;

                        occurrences[t] = occurrences.GetValueOrDefault(t) + 1;
                        if (hasA)
                            touchA[t] = touchA.GetValueOrDefault(t) + 1;
                        if (hasB)
                            touchB[t] = touchB.GetValueOrDefault(t) + 1;
                    }
                }
            }
            var candidates = occurrences.Keys.Where(t => touchA.GetValueOrDefault(t) >= 8 && touchB.GetValueOrDefault(t) >= 8).ToHashSet();

            // Corner votes between known tiles, aggregated over every occurrence.
            var votes = new Dictionary<(int Tile, int Corner), Dictionary<(int Tile, int Corner), long>>();
            var total = new Dictionary<int, long>();
            foreach (var map in maps)
            {
                for (int y = 0; y < map.Height; y++)
                {
                    for (int x = 0; x < map.Width; x++)
                    {
                        int t = map.Tiles[y * map.Width + x];
                        if (candidates.Contains(t) == false)
                            continue;

                        total[t] = total.GetValueOrDefault(t) + 1;
                        for (int c = 0; c < 4; c++)
                        {
                            foreach (var (dx, dy, nc) in Shared[c])
                            {
                                if (map.Contains(x + dx, y + dy) == false)
                                    continue;
                                int n = map.Tiles[(y + dy) * map.Width + x + dx];
                                if (n != a && n != b && candidates.Contains(n) == false)
                                    continue;

                                if (votes.TryGetValue((t, c), out var list) == false)
                                    votes[(t, c)] = list = new Dictionary<(int, int), long>();
                                list[(n, nc)] = list.GetValueOrDefault((n, nc)) + 1;
                            }
                        }
                    }
                }
            }

            var p = new Dictionary<(int Tile, int Corner), double>();
            for (int c = 0; c < 4; c++)
            {
                p[(a, c)] = 0;
                p[(b, c)] = 1;
                foreach (var t in candidates)
                    p[(t, c)] = 0.5;
            }
            for (int iteration = 0; iteration < 30; iteration++)
            {
                var next = new Dictionary<(int, int), double>(p);
                foreach (var ((t, c), list) in votes)
                {
                    double sum = 0, weight = 0;
                    foreach (var ((n, nc), w) in list)
                    {
                        sum += p[(n, nc)] * w;
                        weight += w;
                    }
                    if (weight > 0)
                        next[(t, c)] = sum / weight;
                }
                p = next;
            }

            var set = new TerrainSet { A = a, B = b };
            set.Patterns[0].Add((a, countA, 1));
            set.Patterns[15].Add((b, countB, 1));
            set.PatternOf[a] = 0;
            set.PatternOf[b] = 15;
            foreach (var t in candidates)
            {
                int pattern = 0;
                double confidence = 1;
                for (int c = 0; c < 4; c++)
                {
                    var value = p[(t, c)];
                    if (value > 0.5)
                        pattern |= 1 << c;
                    confidence = Math.Min(confidence, Math.Abs(value * 2 - 1));
                }
                set.Patterns[pattern].Add((t, total.GetValueOrDefault(t), Math.Round(confidence, 2)));
                if (confidence >= MinConfidence)
                    set.PatternOf[t] = pattern;
            }
            foreach (var list in set.Patterns)
                list.Sort((l, r) => r.Count.CompareTo(l.Count));
            return set;
        }

        /// <summary>
        /// New tiles after painting. paint: true makes the cell B, false makes it A, null keeps it (a cell is B when
        /// its tile is B or a transition with a B corner). Cells next to painted ones are re-tiled too; cells whose
        /// tile already fits are kept, and A cells outside the set are left alone. Only tiles below tileCount (the client's
        /// resources) are written.
        /// </summary>
        public List<(int X, int Y, ushort Tile)> Paint(ServerMap map, bool?[] paint, int seed, int tileCount)
        {
            bool IsB(int x, int y)
            {
                if (map.Contains(x, y) == false)
                    return true;
                var i = y * map.Width + x;
                if (paint[i] != null)
                    return paint[i].Value;
                return PatternOf.TryGetValue(map.Tiles[i], out var pattern) && pattern != 0;
            }

            var region = new HashSet<(int X, int Y)>();
            for (int y = 0; y < map.Height; y++)
            {
                for (int x = 0; x < map.Width; x++)
                {
                    if (paint[y * map.Width + x] == null)
                        continue;
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = -1; dx <= 1; dx++)
                        {
                            if (map.Contains(x + dx, y + dy))
                                region.Add((x + dx, y + dy));
                        }
                    }
                }
            }

            var result = new List<(int, int, ushort)>();
            foreach (var (x, y) in region)
            {
                var i = y * map.Width + x;
                var current = map.Tiles[i];
                if (IsB(x, y))
                {
                    int pattern = 0;
                    for (int c = 0; c < 4; c++)
                    {
                        if (Shared[c].All(s => IsB(x + s.Dx, y + s.Dy)))
                            pattern |= 1 << c;
                    }
                    if (PatternOf.TryGetValue(current, out var had) && had == pattern)
                        continue;
                    result.Add((x, y, (ushort)Choose(pattern, x, y, seed, tileCount)));
                }
                else if (paint[i] == false || (PatternOf.TryGetValue(current, out var had) && had != 0))
                {
                    if (current != A)
                        result.Add((x, y, (ushort)A));
                }
            }
            return result;
        }

        /// <summary>
        /// A tile for the pattern, weighted by corpus count; a missing pattern falls back to the nearest one with
        /// fewer B corners first.
        /// </summary>
        private int Choose(int pattern, int x, int y, int seed, int tileCount)
        {
            var best = Enumerable.Range(0, 16)
                .Where(k => Patterns[k].Any(e => e.Confidence >= MinConfidence && e.Tile < tileCount))
                .OrderBy(k => System.Numerics.BitOperations.PopCount((uint)(k ^ pattern)))
                .ThenBy(k => System.Numerics.BitOperations.PopCount((uint)k))
                .First();
            var tiles = Patterns[best].Where(e => e.Confidence >= MinConfidence && e.Tile < tileCount).ToList();
            var sum = (ulong)Math.Max(1, tiles.Sum(e => e.Count));
            var hash = unchecked((ulong)(uint)(x * 73856093 ^ y * 19349663 ^ seed * 83492791)) % sum;
            foreach (var (tile, count, _) in tiles)
            {
                if (hash < (ulong)count)
                    return tile;
                hash -= (ulong)count;
            }
            return tiles[0].Tile;
        }
    }
}
