namespace MultiwaySearchTree
{
    public class SyncService
    {
        private readonly HashSet<string> _routes = new HashSet<string>();
        private readonly Dictionary<string, Mutex> _mutex = new Dictionary<string, Mutex>();

        public MST Root { get; private set; } = new MST();

        public void Sync(string key, Action<MST> fn, MST trans = null)
        {
            if (_mutex.TryGetValue(key, out var m) == false)
            {
                m = new Mutex();
                _mutex.Add(key, m);
            }

            try
            {
                m.WaitOne();

                var isRoot = trans == null;
                var parent = trans ?? Root;
                var current = new MST(parent) { Key = key };

                parent?.Nodes.Add(current);

                if (isRoot)
                {
                    var routes = current.Routes();
                    if (routes.All(route => _routes.Contains(route)) == false)
                    {
                        Root.Nodes.Add(current);
                        foreach (var route in routes)
                            _routes.Add(route);
                    }
                }

                current.Root.AssertDeadLock();

                fn.Invoke(current);
            }
            catch
            {
                throw;
            }
            finally
            {
                m.ReleaseMutex();
            }
        }
    }
}
