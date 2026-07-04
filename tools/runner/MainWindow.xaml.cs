using Runner.ViewModel;
using System.Text;
using System.Windows;
using System.Windows.Controls;

namespace Runner;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Window
{
    private Model.MainWindow _model;
    private ViewModel.MainWindow _vm;

    public MainWindow()
    {
        InitializeComponent();

        _model = Model.MainWindow.Load(this, "settings.json") ?? new Model.MainWindow(this);
        DataContext = _vm = new ViewModel.MainWindow(_model);

        MainTab.SelectionChanged += MainTab_SelectionChanged;
        Loaded += MainWindow_Loaded;
    }

    private async void MainWindow_Loaded(object sender, RoutedEventArgs e)
    {
        if (MainTab.SelectedItem is TabItem { Header: "Home" })
            await _vm.EnsureInfrastructureCheckedAsync();
    }

    private async void MainTab_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (e.Source is not TabControl)
            return;

        if (MainTab.SelectedItem is TabItem { Header: "Home" })
            await _vm.EnsureInfrastructureCheckedAsync();
    }

    private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
    {
        _model.Save("settings.json");
        _vm.Dispose();
    }

    private void TreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        _vm.SelectedProcess = e.NewValue as ServerProcess;
    }

    private void TextBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
    {
        var textBox = sender as TextBox;
        textBox.ScrollToEnd();
    }
}