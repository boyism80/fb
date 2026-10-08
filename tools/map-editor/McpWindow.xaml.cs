using System.Windows;
using MapEditor.ViewModel;

namespace MapEditor
{
    public partial class McpWindow : Window
    {
        public McpWindow(Window owner, MainWindowViewModel editor)
        {
            InitializeComponent();
            Owner = owner;
            DataContext = editor;
        }
    }
}
