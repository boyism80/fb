using System.ComponentModel;
using System.Windows;
using MapEditor.Edit;
using MapEditor.ViewModel;

namespace MapEditor
{
    /// <summary>
    /// Runs the automatic template scan with progress and lets the user pick which candidates to register.
    /// </summary>
    public partial class TemplateScanWindow : Window
    {
        public class Candidate : INotifyPropertyChanged
        {
            public event PropertyChangedEventHandler PropertyChanged;

            public bool Checked { get; set; } = true;
            public MapTemplate Template { get; init; }
        }

        private readonly MainWindowViewModel _editor;
        private CancellationTokenSource _cancel;
        private List<Candidate> _candidates = new List<Candidate>();

        public TemplateScanWindow(MainWindowViewModel editor)
        {
            InitializeComponent();
            _editor = editor;
            DataContext = editor;
            Closing += (s, e) => _cancel?.Cancel();
        }

        private async void OnStart(object sender, RoutedEventArgs e)
        {
            if (int.TryParse(MinCellsBox.Text, out var minCells) == false || int.TryParse(MaxCellsBox.Text, out var maxCells) == false ||
                int.TryParse(MinCountBox.Text, out var minCount) == false ||
                minCells < 1 || maxCells < minCells || minCount < 1)
            {
                MessageBox.Show(this, "옵션 값을 확인하세요.", Title, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            var options = new TemplateScanOptions { MinCells = minCells, MaxCells = maxCells, MinCount = minCount };
            _cancel = new CancellationTokenSource();
            StartButton.IsEnabled = false;
            CancelButton.IsEnabled = true;
            RegisterButton.IsEnabled = false;
            Results.ItemsSource = null;
            var progress = new Progress<(double Done, string Text)>(p =>
            {
                ProgressBar.Value = p.Done;
                ProgressText.Text = p.Text;
            });
            try
            {
                var found = await _editor.ScanTemplates(options, progress, _cancel.Token);
                _candidates = found.Select(t => new Candidate { Template = t }).ToList();
                Results.ItemsSource = _candidates;
                RegisterButton.IsEnabled = _candidates.Count > 0;
                ProgressText.Text = $"새 템플릿 후보 {_candidates.Count}개. 이름을 고치거나 체크를 해제한 뒤 등록하세요.";
            }
            catch (OperationCanceledException)
            {
                ProgressText.Text = "중지했습니다.";
            }
            catch (Exception ex)
            {
                ProgressText.Text = $"스캔 실패: {ex.Message}";
            }
            finally
            {
                StartButton.IsEnabled = true;
                CancelButton.IsEnabled = false;
                _cancel = null;
            }
        }

        private void OnCancel(object sender, RoutedEventArgs e)
        {
            _cancel?.Cancel();
        }

        private void OnCheckAll(object sender, RoutedEventArgs e)
        {
            foreach (var candidate in _candidates)
                candidate.Checked = true;
        }

        private void OnUncheckAll(object sender, RoutedEventArgs e)
        {
            foreach (var candidate in _candidates)
                candidate.Checked = false;
        }

        private void OnRegister(object sender, RoutedEventArgs e)
        {
            var chosen = _candidates.Where(c => c.Checked && string.IsNullOrWhiteSpace(c.Template.Name) == false).Select(c => c.Template).ToList();
            if (chosen.Count == 0)
                return;
            if (MessageBox.Show(this, $"템플릿 {chosen.Count}개를 등록할까요?\n{_editor.TemplatePath}", Title, MessageBoxButton.YesNo, MessageBoxImage.Question) != MessageBoxResult.Yes)
                return;

            _editor.AddTemplates(chosen);
            _candidates = _candidates.Where(c => chosen.Contains(c.Template) == false).ToList();
            Results.ItemsSource = _candidates;
            RegisterButton.IsEnabled = _candidates.Count > 0;
            ProgressText.Text = $"템플릿 {chosen.Count}개 등록. 남은 후보 {_candidates.Count}개.";
        }
    }
}
