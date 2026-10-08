using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using MapEditor.Edit;
using MapEditor.ViewModel;

namespace MapEditor
{
    /// <summary>
    /// Template-based warp position analysis with progress; results stay in the view model so the window can be
    /// closed and reopened.
    /// </summary>
    public partial class TemplateWarpWindow : Window
    {
        private readonly MainWindowViewModel _editor;
        private CancellationTokenSource _cancel;

        public TemplateWarpWindow(MainWindowViewModel editor)
        {
            InitializeComponent();
            _editor = editor;
            DataContext = editor;
            Closing += (s, e) => _cancel?.Cancel();
            if (editor.TemplateWarpIssues.Count > 0)
                ProgressText.Text = $"이전 분석 결과 {editor.TemplateWarpIssues.Count}건";
        }

        private async void OnStart(object sender, RoutedEventArgs e)
        {
            _cancel = new CancellationTokenSource();
            StartButton.IsEnabled = false;
            CancelButton.IsEnabled = true;
            var progress = new Progress<(double Done, string Text)>(p =>
            {
                ProgressBar.Value = p.Done;
                ProgressText.Text = p.Text;
            });
            try
            {
                var issues = await _editor.CheckTemplateWarps(progress, _cancel.Token);
                ProgressBar.Value = 1;
                ProgressText.Text = $"어긋난 워프 {issues.Count}건 (맵 {issues.Select(i => i.MapId).Distinct().Count()}곳, 템플릿 {_editor.Templates.Count}개 기준)";
            }
            catch (OperationCanceledException)
            {
                ProgressText.Text = "중지했습니다.";
            }
            catch (Exception ex)
            {
                ProgressText.Text = $"분석 실패: {ex.Message}";
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

        private void OnDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (IssueGrid.SelectedItem is TemplateWarpIssue issue)
                _ = _editor.FocusTemplateWarpIssue(issue);
        }

        private void OnIssueSelected(object sender, SelectionChangedEventArgs e)
        {
            if (IssueGrid.SelectedItem is TemplateWarpIssue issue && issue.Template != _editor.TemplatePlacesOf)
                _ = _editor.FindTemplatePlaces(issue.Template);
        }

        private void OnPlaceClick(object sender, MouseButtonEventArgs e)
        {
            if (sender is ListBox list && list.SelectedItem is TemplatePlace place)
                _ = _editor.GoToTemplatePlace(place);
        }
    }
}
