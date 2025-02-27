using Runner.ViewModel;
using System.Text;
using System.Windows;

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
}