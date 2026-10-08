using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Threading;
using MapEditor.Edit;
using MapEditor.ViewModel;

namespace MapEditor
{
    /// <summary>
    /// Warp check and map validation results, kept open next to the map so rows can be clicked while editing.
    /// </summary>
    public partial class IssuesWindow : Window
    {
        private readonly MainWindowViewModel _editor;
        private readonly ICollectionView _warpView;
        private bool _countPending;

        public IssuesWindow(Window owner, MainWindowViewModel editor)
        {
            InitializeComponent();
            Owner = owner;
            DataContext = editor;
            _editor = editor;
            _warpView = CollectionViewSource.GetDefaultView(editor.WarpIssues);
            _warpView.Filter = item => item is WarpIssue issue &&
                                       (OpenMapOnly.IsChecked != true || issue.InOpenMap) &&
                                       (CertainOnly.IsChecked != true || issue.Certain);
            editor.WarpIssues.CollectionChanged += OnWarpIssuesChanged;
            editor.PropertyChanged += OnEditorPropertyChanged;
            Closed += (s, e) =>
            {
                editor.WarpIssues.CollectionChanged -= OnWarpIssuesChanged;
                editor.PropertyChanged -= OnEditorPropertyChanged;
                _warpView.Filter = null;
            };
            UpdateCount();
        }

        public void ShowTab(IssueTab tab)
        {
            if (tab == IssueTab.Warps)
                WarpTab.IsSelected = true;
            else
                ValidationTab.IsSelected = true;

            if (IsVisible == false)
                Show();
            if (WindowState == WindowState.Minimized)
                WindowState = WindowState.Normal;
            Activate();
        }

        /// <summary>
        /// A check adds thousands of rows one by one; count them once after the batch instead of per row.
        /// </summary>
        private void OnWarpIssuesChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (_countPending)
                return;

            _countPending = true;
            Dispatcher.BeginInvoke(DispatcherPriority.Background, () =>
            {
                _countPending = false;
                UpdateCount();
            });
        }

        private void OnEditorPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(MainWindowViewModel.Document) && OpenMapOnly.IsChecked == true)
                _warpView.Refresh();
        }

        private void OnFilterChanged(object sender, RoutedEventArgs e)
        {
            _warpView?.Refresh();
            UpdateCount();
        }

        private void UpdateCount()
        {
            var shown = _warpView.Cast<object>().Count();
            var open = _editor.WarpIssues.Count(i => i.InOpenMap);
            WarpCount.Text = $"표시 {shown}건 / 전체 {_editor.WarpIssues.Count}건 (현재 맵 {open}건)";
        }

        private void OnCheckAll(object sender, RoutedEventArgs e)
        {
            foreach (var issue in _warpView.OfType<WarpIssue>().Where(i => i.InOpenMap))
                issue.Checked = true;
        }

        private void OnUncheckAll(object sender, RoutedEventArgs e)
        {
            foreach (var issue in _editor.WarpIssues)
                issue.Checked = false;
        }

        private void OnWarpIssueClick(object sender, MouseButtonEventArgs e)
        {
            // Clicking the check box only toggles it.
            if (e.OriginalSource is DependencyObject source && FindParent<CheckBox>(source) != null)
                return;

            if (WarpGrid.SelectedItem is WarpIssue issue)
                _ = _editor.FocusWarpIssue(issue);
        }

        private void OnValidationDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (sender is ListBox list && list.SelectedItem is ValidationItem item)
                _editor.Jump(item.X, item.Y);
        }

        private static T FindParent<T>(DependencyObject child) where T : DependencyObject
        {
            for (var node = child; node != null; node = node is System.Windows.Media.Visual ? System.Windows.Media.VisualTreeHelper.GetParent(node) : LogicalTreeHelper.GetParent(node))
            {
                if (node is T found)
                    return found;
            }
            return null;
        }
    }
}
