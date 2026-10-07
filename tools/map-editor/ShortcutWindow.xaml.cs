using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using MapEditor.Command;

namespace MapEditor
{
    /// <summary>
    /// Editable copy of one shortcut: up to two gestures.
    /// </summary>
    public class ShortcutRow : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public ShortcutAction Action { get; init; }
        public string First { get; set; } = "";
        public string Second { get; set; } = "";

        public void Load(string text)
        {
            var gestures = ShortcutAction.Split(text);
            First = gestures.Count > 0 ? gestures[0].ToString() : "";
            Second = string.Join(", ", gestures.Skip(1));
        }

        public string Text => string.Join(", ", new[] { First, Second }.Where(t => t != ""));
    }

    /// <summary>
    /// Shortcut editor. Edit returns true when the user saved; the actions' Text is updated then.
    /// </summary>
    public partial class ShortcutWindow : Window
    {
        private readonly List<ShortcutRow> _rows;

        private ShortcutWindow(IEnumerable<ShortcutAction> actions)
        {
            InitializeComponent();
            _rows = actions.Select(a =>
            {
                var row = new ShortcutRow { Action = a };
                row.Load(a.Text);
                return row;
            }).ToList();
            Rows.ItemsSource = _rows;
            UpdateConflicts();
        }

        public static bool Edit(Window owner, IEnumerable<ShortcutAction> actions)
        {
            var window = new ShortcutWindow(actions) { Owner = owner };
            if (window.ShowDialog() != true)
                return false;

            foreach (var row in window._rows)
                row.Action.Text = row.Text;
            return true;
        }

        private void Assign(object sender, string gesture)
        {
            if (sender is not TextBox box || box.DataContext is not ShortcutRow row)
                return;

            if ((string)box.Tag == "First")
                row.First = gesture;
            else
                row.Second = gesture;
            UpdateConflicts();
        }

        private void OnCaptureKey(object sender, KeyEventArgs e)
        {
            var key = e.Key == Key.System ? e.SystemKey : e.Key == Key.ImeProcessed ? e.ImeProcessedKey : e.Key;
            if (Gesture.IsModifierKey(key))
            {
                e.Handled = true;
                return;
            }
            // Plain Tab still moves between fields.
            if (key == Key.Tab && Keyboard.Modifiers == ModifierKeys.None)
                return;

            if (key == Key.Back && Keyboard.Modifiers == ModifierKeys.None)
                Assign(sender, "");
            else
                Assign(sender, new Gesture(Keyboard.Modifiers, key, null).ToString());
            e.Handled = true;
        }

        private void OnCaptureMouse(object sender, MouseButtonEventArgs e)
        {
            if (Gesture.IsGestureButton(e.ChangedButton) == false)
                return;

            Assign(sender, new Gesture(Keyboard.Modifiers, Key.None, e.ChangedButton).ToString());
            e.Handled = true;
        }

        private void OnCaptureFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (sender is TextBox box)
            {
                box.Background = (System.Windows.Media.Brush)FindResource("HighlightBrush");
                Rows.SelectedItem = box.DataContext;
            }
        }

        private void OnCaptureLostFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (sender is TextBox box)
                box.ClearValue(BackgroundProperty);
        }

        private void UpdateConflicts()
        {
            var used = _rows.SelectMany(r => new[] { r.First, r.Second }.Where(t => t != "").Select(t => (Gesture: t, Row: r)))
                            .GroupBy(u => u.Gesture)
                            .Where(g => g.Select(u => u.Row).Distinct().Count() > 1)
                            .Select(g => $"{g.Key}: {string.Join(", ", g.Select(u => u.Row.Action.Label).Distinct())}")
                            .ToList();
            Conflicts.Text = used.Count == 0 ? "" : "겹치는 단축키 (위쪽 항목이 실행됨): " + string.Join(" / ", used);
        }

        private void OnResetSelected(object sender, RoutedEventArgs e)
        {
            if (Rows.SelectedItem is ShortcutRow row)
                row.Load(row.Action.Default);
            UpdateConflicts();
        }

        private void OnResetAll(object sender, RoutedEventArgs e)
        {
            foreach (var row in _rows)
                row.Load(row.Action.Default);
            UpdateConflicts();
        }

        private void OnSave(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }
    }
}
