using System.Collections.Concurrent;
using System.IO;
using MapEditor.Format;

namespace MapEditor.Edit
{
    /// <summary>
    /// Every server map (and, on demand, its .block) read once for usage statistics and learning; about 60 MB.
    /// </summary>
    public static class MapCorpus
    {
        private static Task<Dictionary<int, ServerMap>> _maps;
        private static string _directory;
        private static readonly ConcurrentDictionary<int, BlockFile> _blocks = new ConcurrentDictionary<int, BlockFile>();
        private static readonly object _lock = new object();

        public static Task<Dictionary<int, ServerMap>> Load(string directory)
        {
            lock (_lock)
            {
                if (_maps == null || _directory != directory || _maps.IsFaulted)
                {
                    _directory = directory;
                    _blocks.Clear();
                    _maps = Task.Run(() =>
                    {
                        var maps = new ConcurrentDictionary<int, ServerMap>();
                        Parallel.ForEach(Directory.GetFiles(directory, "*.map"), path =>
                        {
                            if (int.TryParse(Path.GetFileNameWithoutExtension(path), out var id))
                                maps[id] = ServerMap.Read(path);
                        });
                        return new Dictionary<int, ServerMap>(maps);
                    });
                }
                return _maps;
            }
        }

        public static BlockFile Blocks(string directory, int id)
        {
            return _blocks.GetOrAdd(id, key => BlockFile.Read(Path.Combine(directory, $"{key:000000}.block")));
        }

        /// <summary>
        /// Keeps a loaded corpus in step with a saved (map != null) or deleted (map == null) map.
        /// </summary>
        public static void Update(string directory, int id, ServerMap map)
        {
            Task<Dictionary<int, ServerMap>> maps;
            lock (_lock)
            {
                if (_maps == null || _directory != directory)
                    return;
                maps = _maps;
            }
            _blocks.TryRemove(id, out _);
            maps.ContinueWith(task =>
            {
                if (task.IsCompletedSuccessfully == false)
                    return;
                lock (task.Result)
                {
                    if (map == null)
                        task.Result.Remove(id);
                    else
                        task.Result[id] = map;
                }
            }, TaskContinuationOptions.ExecuteSynchronously);
        }
    }
}
