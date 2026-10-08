using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using MapEditor.Command;
using MapEditor.Edit;

namespace MapEditor.ViewModel
{
    public partial class MainWindowViewModel
    {
        public event Action NewMapRequested;

        /// <summary>
        /// Argument: the tab to save under a new map id.
        /// </summary>
        public event Action<MapDocument> SaveAsRequested;

        public RelayCommand NewMapCommand { get; private set; }
        public RelayCommand SaveAsCommand { get; private set; }
        public RelayCommand SaveAllCommand { get; private set; }
        public RelayCommand CloseTabCommand { get; private set; }
        public RelayCommand CloseAllTabsCommand { get; private set; }

        public void RequestSaveAs(MapDocument document)
        {
            if (document != null && CanOpen)
                SaveAsRequested?.Invoke(document);
        }

        /// <summary>
        /// Parameter: the map list's SelectedItems (MapEntry), or null for SelectedMap.
        /// </summary>
        public RelayCommand<System.Collections.IList> DeleteMapsCommand { get; private set; }

        public List<string> MapSheets => _spawns?.MapSheets ?? new List<string>();
        public string MapSheet(int id) => _spawns?.MapSheet(id);

        private void InitializeMapCommands()
        {
            NewMapCommand = new RelayCommand(_ => NewMapRequested?.Invoke());
            SaveAsCommand = new RelayCommand(_ => RequestSaveAs(Document));
            SaveAllCommand = new RelayCommand(_ => SaveAll());
            CloseTabCommand = new RelayCommand(_ => CloseDocument(Document));
            CloseAllTabsCommand = new RelayCommand(_ => CloseDocuments(OpenDocuments));
            DeleteMapsCommand = new RelayCommand<System.Collections.IList>(items =>
            {
                var ids = items?.OfType<MapEntry>().Select(m => m.Id).ToList() ?? new List<int>();
                if (ids.Count == 0 && SelectedMap != null)
                    ids.Add(SelectedMap.Id);
                DeleteMaps(ids);
            });
        }

        /// <summary>
        /// After a confirmation: removes the maps' map.xlsx rows and spawn/warp groups, moves their .map/.block
        /// files (and backups) to the recycle bin and closes the open map when it is one of them. Returns the
        /// number of deleted maps; 0 when cancelled.
        /// </summary>
        public int DeleteMaps(IReadOnlyCollection<int> ids)
        {
            if (_spawns == null || Busy)
                throw new InvalidOperationException("the editor is still loading");

            var entries = Maps.Where(m => ids.Contains(m.Id)).ToList();
            if (entries.Count == 0)
            {
                StatusText = "삭제할 맵을 선택하세요.";
                return 0;
            }

            var deleted = entries.Select(e => e.Id).ToHashSet();
            var incoming = _spawns.ReadAllWarps()
                                  .Where(w => deleted.Contains(w.Key) == false)
                                  .SelectMany(w => w.Value.Where(warp => warp.DestMap is int dest && deleted.Contains(dest)).Select(_ => w.Key))
                                  .ToList();
            var names = string.Join("\n", entries.Take(15).Select(e => $"  {e.Label}")) + (entries.Count > 15 ? $"\n  ... 외 {entries.Count - 15}개" : "");
            var message = $"맵 {entries.Count}개를 삭제할까요?\n\n{names}\n\n" +
                          ".map / .block 파일은 휴지통으로 이동하고 map.xlsx 행과 npc_spawn, mob_spawn, warp 항목을 지웁니다.";
            var closing = OpenDocuments.Where(d => deleted.Contains(d.Id)).ToList();
            if (closing.Count > 0)
                message += closing.Any(d => d.Dirty) ? "\n\n열려 있는 맵이 포함되어 있으며 저장하지 않은 변경 사항은 버려집니다." : "\n\n열려 있는 맵이 포함되어 있어 탭이 닫힙니다.";
            if (incoming.Count > 0)
                message += $"\n\n다른 맵 {incoming.Distinct().Count()}곳에서 이 맵으로 가는 워프 {incoming.Count}개는 그대로 남습니다 (워프 검사로 찾을 수 있음).";
            if (MessageBox.Show(message, "맵 삭제", MessageBoxButton.YesNo, MessageBoxImage.Warning, MessageBoxResult.No) != MessageBoxResult.Yes)
                return 0;

            try
            {
                _spawns.RemoveMaps(deleted);
            }
            catch (IOException e)
            {
                StatusText = $"맵 삭제 실패 (엑셀에서 파일을 열고 있는지 확인): {e.Message}";
                return 0;
            }

            if (Document != null && deleted.Contains(Document.Id))
                Activate(OpenDocuments.FirstOrDefault(d => deleted.Contains(d.Id) == false));
            foreach (var document in closing)
                OpenDocuments.Remove(document);

            var directory = Settings.MapDirectory;
            var files = deleted.SelectMany(id => new[] { ".map", ".block", ".map.bak", ".block.bak" }.Select(ext => Path.Combine(directory, $"{id:000000}{ext}")))
                               .Where(File.Exists)
                               .ToList();
            var recycled = files.Count == 0 || Recycle(files);
            foreach (var entry in entries)
            {
                Maps.Remove(entry);
                MapCorpus.Update(directory, entry.Id, null);
            }
            OnPropertyChanged(nameof(MapChoices));
            ScheduleDoorReferences();
            StatusText = recycled
                ? $"맵 {entries.Count}개 삭제 (파일은 휴지통으로 이동)"
                : $"맵 {entries.Count}개를 map.xlsx에서 지웠지만 파일을 휴지통으로 옮기지 못했습니다: {directory}";
            return entries.Count;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        private struct ShFileOperation
        {
            public IntPtr Window;
            public uint Function;
            public string From;
            public string To;
            public ushort Flags;
            public bool Aborted;
            public IntPtr NameMappings;
            public string ProgressTitle;
        }

        [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
        private static extern int SHFileOperation(ref ShFileOperation operation);

        /// <summary>
        /// Moves files to the recycle bin without shell dialogs.
        /// </summary>
        private static bool Recycle(IEnumerable<string> paths)
        {
            const uint delete = 3;
            const ushort allowUndo = 0x40, noConfirmation = 0x10, silent = 0x4, noErrorUi = 0x400;
            var operation = new ShFileOperation
            {
                Function = delete,
                From = string.Join("\0", paths.Select(Path.GetFullPath)) + "\0\0",
                Flags = allowUndo | noConfirmation | silent | noErrorUi,
            };
            return SHFileOperation(ref operation) == 0 && operation.Aborted == false;
        }
    }
}
