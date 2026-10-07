using System.Windows.Input;

namespace MapEditor.Command
{
    public class RelayCommand<T> : ICommand
    {
        private readonly Action<T> _action;

        public event EventHandler CanExecuteChanged = (sender, e) => { };

        public RelayCommand(Action<T> action)
        {
            _action = action;
        }

        public bool CanExecute(object parameter)
        {
            return true;
        }

        public void Execute(object parameter)
        {
            _action(parameter is T value ? value : default);
        }
    }

    public class RelayCommand : RelayCommand<object>
    {
        public RelayCommand(Action<object> action) : base(action)
        { }
    }
}
