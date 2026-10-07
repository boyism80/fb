using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using MapEditor.Asset;
using MapEditor.Table;

namespace MapEditor
{
    /// <summary>
    /// Searchable id/name list. Show returns the chosen entry or null.
    /// </summary>
    public partial class PickerWindow : Window
    {
        private readonly IReadOnlyList<NameEntry> _entries;

        public NameEntry Selected { get; private set; }

        /// <summary>
        /// Set for npc/mob lists so each row shows its sprite.
        /// </summary>
        public ClientAssets Assets { get; }
        public bool HasImages => Assets?.HasMonsters == true;

        private PickerWindow(string title, IReadOnlyList<NameEntry> entries, ClientAssets assets)
        {
            Assets = assets;
            InitializeComponent();
            Title = title;
            _entries = entries;
            Items.ItemsSource = entries;
            Loaded += (s, e) => Query.Focus();
        }

        public static NameEntry Show(string title, IReadOnlyList<NameEntry> entries, ClientAssets assets = null)
        {
            var window = new PickerWindow(title, entries, assets) { Owner = Application.Current.MainWindow };
            return window.ShowDialog() == true ? window.Selected : null;
        }

        private void OnQueryChanged(object sender, TextChangedEventArgs e)
        {
            var query = Query.Text.Trim();
            Items.ItemsSource = query == ""
                ? _entries
                : _entries.Where(entry => entry.Label.Contains(query, StringComparison.OrdinalIgnoreCase)).ToList();
            Items.SelectedIndex = 0;
        }

        private void OnQueryKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Down && Items.Items.Count > 0)
            {
                Items.SelectedIndex = Math.Min(Items.Items.Count - 1, Items.SelectedIndex + 1);
                Items.ScrollIntoView(Items.SelectedItem);
                e.Handled = true;
            }
            else if (e.Key == Key.Up && Items.Items.Count > 0)
            {
                Items.SelectedIndex = Math.Max(0, Items.SelectedIndex - 1);
                Items.ScrollIntoView(Items.SelectedItem);
                e.Handled = true;
            }
        }

        private void OnOk(object sender, RoutedEventArgs e)
        {
            Selected = Items.SelectedItem as NameEntry;
            if (Selected != null)
                DialogResult = true;
        }
    }
}
