using System.Windows;
using MapEditor.ViewModel;

namespace MapEditor
{
    /// <summary>
    /// Asks which kinds of a mixed selection to copy. Show returns the chosen kinds, or null when cancelled.
    /// </summary>
    public partial class CopyWindow : Window
    {
        public class Option
        {
            public CopyKind Kind { get; init; }
            public string Label { get; init; }
            public bool Checked { get; set; } = true;
        }

        private readonly List<Option> _options;
        private HashSet<CopyKind> _chosen;

        private CopyWindow(List<Option> options)
        {
            InitializeComponent();
            _options = options;
            Options.ItemsSource = options;
        }

        public static HashSet<CopyKind> Show(IEnumerable<(CopyKind Kind, string Label)> options)
        {
            var window = new CopyWindow(options.Select(o => new Option { Kind = o.Kind, Label = o.Label }).ToList())
            {
                Owner = Application.Current.MainWindow,
            };
            return window.ShowDialog() == true ? window._chosen : null;
        }

        private void OnAll(object sender, RoutedEventArgs e)
        {
            _chosen = _options.Select(o => o.Kind).ToHashSet();
            DialogResult = true;
        }

        private void OnChecked(object sender, RoutedEventArgs e)
        {
            _chosen = _options.Where(o => o.Checked).Select(o => o.Kind).ToHashSet();
            if (_chosen.Count > 0)
                DialogResult = true;
        }
    }
}
