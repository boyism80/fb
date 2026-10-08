using System.Windows;
using MapEditor.ViewModel;

namespace MapEditor
{
    /// <summary>
    /// door / door_pair table editing (door.xlsx). Placing doors on the map stays in the main window.
    /// </summary>
    public partial class DoorEditorWindow : Window
    {
        public DoorEditorWindow(Window owner, MainWindowViewModel editor)
        {
            InitializeComponent();
            Owner = owner;
            DataContext = editor;
        }
    }
}
