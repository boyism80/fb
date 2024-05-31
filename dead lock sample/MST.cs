namespace MultiwaySearchTree
{
    public class RaceConditionResult
    {
        public string Sequence1 { get; set; }
        public string Sequence2 { get; set; }
    }

    public class MST
    {
        public MST Parent { get; private set; }
        public List<MST> Nodes = new List<MST>();
        public string Key { get; set; }

        public MST(MST parent = null)
        {
            Parent = parent;
        }

        public MST Root
        {
            get
            {
                var node = this;
                while (node.Parent != null)
                    node = node.Parent;

                return node;
            }
        }

        public IEnumerable<string> Routes()
        {
            var queue = new Queue<List<MST>>();
            if (Key != null)
            {
                queue.Enqueue(new List<MST> { this });
            }
            else
            {
                foreach (var node in Nodes)
                {
                    queue.Enqueue(new List<MST> { node });
                }
            }

            var visit = new HashSet<string>();
            while (queue.TryDequeue(out var root))
            {
                var last = root.Last();

                if (last.Nodes.Count == 0)
                {
                    var keys = string.Join('-', root.Select(x => x.Key));
                    if (visit.Contains(keys))
                        continue;

                    visit.Add(keys);
                    yield return keys;
                }
                else
                {
                    foreach (var child in last.Nodes)
                    {
                        var sequence = new List<MST>(root);
                        sequence.Add(child);
                        queue.Enqueue(sequence);
                    }
                }
            }
        }

        private static void AssertCirculatedLock(string sequence)
        {
            if (sequence.Split('-').GroupBy(x => x).Any(x => x.Count() > 1))
                throw new Exception($"Circulated lock detected : {string.Join('-', sequence)}");
        }

        private static int Index(List<string> keys, string key)
        {
            for(int i = 0; i < keys.Count; i++)
            {
                if (keys[i] == key)
                    return i;
            }

            return -1;
        }

        private static bool AssertDeadLock(string sequence1, string sequence2)
        {
            var keys1 = sequence1.Split('-').ToList();
            var keys2 = sequence2.Split('-').ToList();
            for (int i1 = 0; i1 < keys1.Count - 1; i1++)
            {
                for (int i2 = i1 + 1; i2 < keys1.Count; i2++)
                { 
                    var begin = keys1[i1];
                    var end = keys1[i2];

                    var offset1 = Index(keys2, begin);
                    if (offset1 == -1)
                        continue;

                    var offset2 = Index(keys2, end);
                    if (offset2 == -1)
                        continue;

                    if (offset1 > offset2)
                        return true;
                }
            }

            return false;
        }

        private static void AssertDeadLock(List<string> sequences)
        { 
            var result = new List<RaceConditionResult>();

            for(int i1 = 0; i1 < sequences.Count - 1; i1++)
            {
                for(int i2 = i1; i2 < sequences.Count; i2++)
                {
                    var sequence1 = sequences[i1];
                    var sequence2 = sequences[i2];

                    if (AssertDeadLock(sequence1, sequence2))
                    {
                        result.Add(new RaceConditionResult
                        {
                            Sequence1 = sequence1,
                            Sequence2 = sequence2
                        });
                    }
                }
            }

            if (result.Count > 0)
            {
                var matchedList = result.Select(x =>
                {
                    var sequence1 = string.Join('-', x.Sequence1);
                    var sequence2 = string.Join('-', x.Sequence2);
                    return $"{sequence1} <=> {sequence2}";
                });

                throw new Exception($"DeadLock detected : {string.Join(Environment.NewLine, matchedList)}");
            }
        }

        public void AssertDeadLock()
        {
            var errors = new List<Exception>();
            var sequences = Routes().ToList();
            try
            {
                foreach (var sequence in sequences)
                {
                    AssertCirculatedLock(sequence);
                }
            }
            catch (Exception e)
            {
                errors.Add(e);
            }

            try
            {
                AssertDeadLock(sequences);
            }
            catch (Exception e)
            {
                errors.Add(e);
            }

            if (errors.Count == 0)
                return;
            else if (errors.Count > 1)
                throw new AggregateException(errors);
            else
                throw errors[0];
        }
    }
}
