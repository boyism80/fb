using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace Runner.Behavior
{
    public static class DataGridScrollBehavior
    {
        public static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.RegisterAttached(
                "IsEnabled",
                typeof(bool),
                typeof(DataGridScrollBehavior),
                new PropertyMetadata(false, OnIsEnabledChanged));

        public static bool GetIsEnabled(DependencyObject obj) => (bool)obj.GetValue(IsEnabledProperty);

        public static void SetIsEnabled(DependencyObject obj, bool value) => obj.SetValue(IsEnabledProperty, value);

        private static void OnIsEnabledChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not DataGrid grid)
                return;

            if ((bool)e.NewValue)
                grid.PreviewMouseWheel += OnPreviewMouseWheel;
            else
                grid.PreviewMouseWheel -= OnPreviewMouseWheel;
        }

        private static void OnPreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (sender is not DataGrid grid)
                return;

            var gridScroll = FindChildScrollViewer(grid);
            if (gridScroll != null && gridScroll.ScrollableHeight > 0)
            {
                var offset = gridScroll.VerticalOffset;
                var max = gridScroll.ScrollableHeight;

                if (e.Delta < 0 && offset < max)
                {
                    gridScroll.ScrollToVerticalOffset(offset - e.Delta);
                    e.Handled = true;
                    return;
                }

                if (e.Delta > 0 && offset > 0)
                {
                    gridScroll.ScrollToVerticalOffset(offset - e.Delta);
                    e.Handled = true;
                    return;
                }
            }

            var parent = FindParentScrollViewer(grid);
            if (parent == null)
                return;

            e.Handled = true;
            parent.ScrollToVerticalOffset(parent.VerticalOffset - e.Delta);
        }

        private static ScrollViewer FindChildScrollViewer(DependencyObject parent)
        {
            for (var i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++)
            {
                var child = VisualTreeHelper.GetChild(parent, i);
                if (child is ScrollViewer scrollViewer)
                    return scrollViewer;

                var nested = FindChildScrollViewer(child);
                if (nested != null)
                    return nested;
            }

            return null;
        }

        private static ScrollViewer FindParentScrollViewer(DependencyObject child)
        {
            for (var parent = VisualTreeHelper.GetParent(child); parent != null; parent = VisualTreeHelper.GetParent(parent))
            {
                if (parent is ScrollViewer scrollViewer)
                    return scrollViewer;
            }

            return null;
        }
    }
}
