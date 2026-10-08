using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.IO;
using MapEditor.Format;
using MapEditor.Table;

namespace MapEditor.Edit
{
    [Flags]
    public enum EditLayer
    {
        None = 0,
        Tile = 1,
        Object = 2,
        Block = 4,
    }

    public struct CellValue
    {
        public ushort Tile;
        public ushort Object;
        public bool Block;
    }

    public struct CellChange
    {
        public int X;
        public int Y;
        public CellValue Before;
        public CellValue After;
    }

    /// <summary>
    /// One reversible edit inside an undo step.
    /// </summary>
    public interface IEditOp
    {
        void Undo();
        void Redo();
    }

    public class CellsOp : IEditOp
    {
        private readonly MapDocument _document;
        public List<CellChange> Changes { get; } = new List<CellChange>();

        public CellsOp(MapDocument document)
        {
            _document = document;
        }

        public void Undo()
        {
            // A merged stroke can touch a cell twice; reverse order restores the earliest value.
            for (int i = Changes.Count - 1; i >= 0; i--)
                _document.Set(Changes[i].X, Changes[i].Y, Changes[i].Before);
            _document.RaiseCellsChanged(Changes.Select(c => (c.X, c.Y)).ToList());
        }

        public void Redo()
        {
            foreach (var change in Changes)
                _document.Set(change.X, change.Y, change.After);
            _document.RaiseCellsChanged(Changes.Select(c => (c.X, c.Y)).ToList());
        }
    }

    public class PropertyOp : IEditOp
    {
        public object Target { get; init; }
        public string Property { get; init; }
        public object Before { get; init; }
        public object After { get; init; }

        public void Undo() => Target.GetType().GetProperty(Property).SetValue(Target, Before);
        public void Redo() => Target.GetType().GetProperty(Property).SetValue(Target, After);
    }

    public class ListOp<T> : IEditOp
    {
        public ObservableCollection<T> List { get; init; }
        public T Item { get; init; }
        public int Index { get; init; }
        public bool Added { get; init; }

        public void Undo()
        {
            if (Added)
                List.Remove(Item);
            else
                List.Insert(Math.Min(Index, List.Count), Item);
        }

        public void Redo()
        {
            if (Added)
                List.Insert(Math.Min(Index, List.Count), Item);
            else
                List.Remove(Item);
        }
    }

    /// <summary>
    /// One open map: server .map + .block, its spawns and the undo history.
    /// Cell edits go through Apply; spawn edits are recorded from their change events, so UI, canvas drags and MCP
    /// all land in the same history.
    /// </summary>
    public class MapDocument : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Raised with the changed cells after an edit, undo or redo.
        /// </summary>
        public event Action<IReadOnlyList<(int X, int Y)>> CellsChanged;

        private readonly Stack<List<IEditOp>> _undo = new Stack<List<IEditOp>>();
        private readonly Stack<List<IEditOp>> _redo = new Stack<List<IEditOp>>();
        private List<IEditOp> _group;
        private int _groupDepth;
        private bool _replaying;

        public int Id { get; init; }
        public string Name { get; init; } = "";
        public ServerMap Map { get; init; }
        public BlockFile Blocks { get; init; }
        public int Width => Map.Width;
        public int Height => Map.Height;

        public bool MapDirty { get; private set; }
        public bool NpcDirty { get; set; }
        public bool MobDirty { get; set; }
        public bool WarpDirty { get; set; }
        public bool Dirty => MapDirty || NpcDirty || MobDirty || WarpDirty;
        public bool CanUndo => _undo.Count > 0;

        /// <summary>
        /// The newest undo step; compare references to tell whether anything was edited since.
        /// </summary>
        public object LastStep => _undo.Count > 0 ? _undo.Peek() : null;
        public bool CanRedo => _redo.Count > 0;
        public string Title => Dirty ? $"{Id:000000} {Name} *" : $"{Id:000000} {Name}";

        /// <summary>
        /// View of this tab while another tab is active: center in cells (null = never shown) and zoom.
        /// </summary>
        [PropertyChanged.DoNotNotify]
        public (double X, double Y)? ViewCenter { get; set; }
        [PropertyChanged.DoNotNotify]
        public double Zoom { get; set; } = 1;

        public List<MapDoor> Doors { get; set; } = new List<MapDoor>();
        public ObservableCollection<NpcSpawn> Npcs { get; private set; } = new ObservableCollection<NpcSpawn>();
        public ObservableCollection<MobSpawn> Mobs { get; private set; } = new ObservableCollection<MobSpawn>();
        public ObservableCollection<WarpEntry> Warps { get; private set; } = new ObservableCollection<WarpEntry>();

        public static MapDocument Open(string mapDirectory, int id, string name)
        {
            return new MapDocument
            {
                Id = id,
                Name = name,
                Map = ServerMap.Read(Path.Combine(mapDirectory, $"{id:000000}.map")),
                Blocks = BlockFile.Read(Path.Combine(mapDirectory, $"{id:000000}.block")),
            };
        }

        /// <summary>
        /// Takes the spawn lists and starts recording their edits. Call on the UI thread.
        /// </summary>
        public void Attach(List<NpcSpawn> npcs, List<MobSpawn> mobs, List<WarpEntry> warps)
        {
            Npcs = new ObservableCollection<NpcSpawn>(npcs);
            Mobs = new ObservableCollection<MobSpawn>(mobs);
            Warps = new ObservableCollection<WarpEntry>(warps);
            Track(Npcs, () => NpcDirty = true);
            Track(Mobs, () => MobDirty = true);
            Track(Warps, () => WarpDirty = true);
        }

        private void Track<T>(ObservableCollection<T> items, Action markDirty) where T : Entity
        {
            void OnEdited(Entity entity, string property, object before, object after)
            {
                markDirty();
                Record(new PropertyOp { Target = entity, Property = property, Before = before, After = after });
            }

            foreach (var item in items)
                item.Edited += OnEdited;

            items.CollectionChanged += (s, e) =>
            {
                markDirty();
                if (e.Action == NotifyCollectionChangedAction.Add)
                {
                    foreach (T item in e.NewItems)
                    {
                        item.Edited += OnEdited;
                        Record(new ListOp<T> { List = items, Item = item, Index = e.NewStartingIndex, Added = true });
                    }
                }
                else if (e.Action == NotifyCollectionChangedAction.Remove)
                {
                    foreach (T item in e.OldItems)
                    {
                        item.Edited -= OnEdited;
                        Record(new ListOp<T> { List = items, Item = item, Index = e.OldStartingIndex, Added = false });
                    }
                }
            };
        }

        private void Record(IEditOp op)
        {
            if (_replaying)
                return;

            if (_group != null)
                _group.Add(op);
            else
                _undo.Push(new List<IEditOp> { op });
            _redo.Clear();
        }

        /// <summary>
        /// Every edit until the returned scope is disposed becomes one undo step (moving many things at once).
        /// </summary>
        public IDisposable Group()
        {
            if (_groupDepth++ == 0)
                _group = new List<IEditOp>();
            return new GroupScope(this);
        }

        private void EndGroup()
        {
            if (--_groupDepth > 0)
                return;

            if (_group.Count > 0)
                _undo.Push(_group);
            _group = null;
        }

        private class GroupScope : IDisposable
        {
            private MapDocument _document;

            public GroupScope(MapDocument document)
            {
                _document = document;
            }

            public void Dispose()
            {
                _document?.EndGroup();
                _document = null;
            }
        }

        public CellValue Get(int x, int y)
        {
            var i = y * Map.Width + x;
            return new CellValue { Tile = Map.Tiles[i], Object = Map.Objects[i], Block = Blocks.Contains(x, y) };
        }

        internal void Set(int x, int y, CellValue value)
        {
            var i = y * Map.Width + x;
            Map.Tiles[i] = value.Tile;
            Map.Objects[i] = value.Object;
            Blocks.Set(x, y, value.Block);
        }

        internal void RaiseCellsChanged(IReadOnlyList<(int X, int Y)> cells)
        {
            MapDirty = true;
            CellsChanged?.Invoke(cells);
        }

        /// <summary>
        /// Applies new cell values as one undo step. Cells outside the map and no-op cells are skipped.
        /// With merge, the cells join the previous step (one brush stroke = one undo).
        /// </summary>
        public int Apply(IEnumerable<(int X, int Y, CellValue Value)> cells, bool merge = false)
        {
            var op = new CellsOp(this);
            var seen = new HashSet<(int, int)>();
            foreach (var (x, y, value) in cells)
            {
                if (Map.Contains(x, y) == false || seen.Add((x, y)) == false)
                    continue;

                var before = Get(x, y);
                if (before.Tile == value.Tile && before.Object == value.Object && before.Block == value.Block)
                    continue;

                op.Changes.Add(new CellChange { X = x, Y = y, Before = before, After = value });
            }
            if (op.Changes.Count == 0)
                return 0;

            foreach (var change in op.Changes)
                Set(change.X, change.Y, change.After);

            if (merge && _group == null && _undo.Count > 0)
            {
                _undo.Peek().Add(op);
                _redo.Clear();
            }
            else
            {
                Record(op);
            }
            RaiseCellsChanged(op.Changes.Select(c => (c.X, c.Y)).ToList());
            return op.Changes.Count;
        }

        public void Undo()
        {
            if (_undo.Count == 0)
                return;

            var step = _undo.Pop();
            _replaying = true;
            try
            {
                for (int i = step.Count - 1; i >= 0; i--)
                    step[i].Undo();
            }
            finally
            {
                _replaying = false;
            }
            _redo.Push(step);
        }

        public void Redo()
        {
            if (_redo.Count == 0)
                return;

            var step = _redo.Pop();
            _replaying = true;
            try
            {
                foreach (var op in step)
                    op.Redo();
            }
            finally
            {
                _replaying = false;
            }
            _undo.Push(step);
        }

        /// <summary>
        /// Writes .map and .block through temp files and keeps the previous files as *.bak.
        /// </summary>
        public void SaveMap(string mapDirectory)
        {
            WriteAtomic(Path.Combine(mapDirectory, $"{Id:000000}.map"), Map.ToBytes());
            WriteAtomic(Path.Combine(mapDirectory, $"{Id:000000}.block"), Blocks.ToBytes());
            MapDirty = false;
        }

        /// <summary>
        /// Writes through a .tmp file and keeps the previous file as .bak.
        /// </summary>
        public static void WriteAtomic(string path, byte[] bytes)
        {
            var temp = path + ".tmp";
            File.WriteAllBytes(temp, bytes);
            if (File.Exists(path))
                File.Replace(temp, path, path + ".bak");
            else
                File.Move(temp, path);
        }
    }
}
