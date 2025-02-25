using Microsoft.Win32;
using Runner.Command;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows.Input;

namespace Runner.ViewModel
{
    public class MySqlConnection : INotifyPropertyChanged
    {
        public Model.MySqlConnection Model { get; private set; }

        public string Name
        {
            get => Model.Name;
            set => Model.Name = value;
        }
        public string IP
        {
            get => Model.IP;
            set => Model.IP = value;
        }
        public ushort Port
        {
            get => Model.Port;
            set => Model.Port = value;
        }

        public MySqlConnection(Model.MySqlConnection model)
        {
            Model = model;
        }

        public event PropertyChangedEventHandler PropertyChanged;
    }

    public class RedisConnection : INotifyPropertyChanged
    {
        public Model.RedisConnection Model { get; private set; }
        public string IP
        {
            get => Model.IP;
            set => Model.IP = value;
        }
        public ushort Port
        {
            get => Model.Port;
            set => Model.Port = value;
        }

        public RedisConnection(Model.RedisConnection model)
        {
            Model = model;
        }

        public event PropertyChangedEventHandler PropertyChanged;
    }

    public class GatewayConfig
    {
        public Model.GatewayConfig Model { get; private set; }
        public ushort Port
        {
            get => Model.Port;
            set => Model.Port = value;
        }

        public GatewayConfig(Model.GatewayConfig model)
        {
            Model = model;
        }
    }

    public class LoginConfig
    {
        public Model.LoginConfig Model { get; private set; }
        public string Name
        {
            get => Model.Name;
            set => Model.Name = value;
        }
        public string Desc
        {
            get => Model.Desc;
            set => Model.Desc = value;
        }
        public ushort Port
        {
            get => Model.Port;
            set => Model.Port = value;
        }

        public LoginConfig(Model.LoginConfig model)
        {
            Model = model;
        }
    }

    public class GameConfig
    {
        public Model.GameConfig Model { get; private set; }
        public int ID
        {
            get => Model.ID;
            set => Model.ID = value;
        }
        public ushort Port
        {
            get => Model.Port;
            set => Model.Port = value;
        }

        public GameConfig(Model.GameConfig model)
        {
            Model = model;
        }
    }

    public class MainWindow : INotifyPropertyChanged
    {
        public Model.MainWindow Model { get; private set; }

        public ObservableCollection<MySqlConnection> MySQL { get; set; } = new ObservableCollection<MySqlConnection>();
        public ObservableCollection<RedisConnection> Redis { get; set; } = new ObservableCollection<RedisConnection>();
        public string RabbitMqIP
        {
            get => Model.RabbitMq.IP;
            set => Model.RabbitMq.IP = value;
        }
        public ushort RabbitMqPort
        {
            get => Model.RabbitMq.Port;
            set => Model.RabbitMq.Port = value;
        }
        public ushort InternalPort
        {
            get => Model.Internal.Port;
            set => Model.Internal.Port = value;
        }
        public GatewayConfig Gateway { get; set; }
        public ObservableCollection<LoginConfig> Login { get; set; } = new ObservableCollection<LoginConfig>();
        public ObservableCollection<GameConfig> Game { get; set; } = new ObservableCollection<GameConfig>();
        public string GatewayFile
        {
            get => Model.GatewayFile;
            set => Model.GatewayFile = value;
        }
        public string LoginFile
        {
            get => Model.LoginFile;
            set => Model.LoginFile = value;
        }
        public string GameFile
        {
            get => Model.GameFile;
            set => Model.GameFile = value;
        }
        public string InternalFile
        {
            get => Model.InternalFile;
            set => Model.InternalFile = value;
        }
        public bool IsEnabled { get; set; } = true;

        public ICommand SetMinimizeCommand { get; private set; }
        public ICommand SetMaximizeCommand { get; private set; }
        public ICommand CloseCommand { get; private set; }
        public ICommand NewMySQL { get; private set; }
        public ICommand NewRedis { get; private set; }
        public ICommand NewLogin { get; private set; }
        public ICommand NewGame { get; private set; }
        public ICommand FindGatewayFile { get; private set; }
        public ICommand FindLoginFile { get; private set; }
        public ICommand FindGameFile { get; private set; }
        public ICommand FindInternalFile { get; private set; }
        public ICommand BuildCommand { get; private set; }
        public ICommand RunCommand { get; private set; }

        public MainWindow(Model.MainWindow model)
        {
            Model = model;

            foreach (var mySqlConnection in Model.MySQL)
            {
                MySQL.Add(new MySqlConnection(mySqlConnection));
            }
            MySQL.CollectionChanged += MySQL_CollectionChanged;

            foreach (var redis in Model.Redis)
            {
                Redis.Add(new RedisConnection(redis));
            }
            Redis.CollectionChanged += Redis_CollectionChanged;

            foreach (var login in Model.Login)
            {
                Login.Add(new LoginConfig(login));
            }
            Login.CollectionChanged += Login_CollectionChanged;

            foreach (var game in Model.Game)
            {
                Game.Add(new GameConfig(game));
            }
            Game.CollectionChanged += Game_CollectionChanged;

            Gateway = new GatewayConfig(Model.Gateway);
            SetMinimizeCommand = new RelayCommand(OnSetMinimize);
            SetMaximizeCommand = new RelayCommand(OnSetMaximize);
            CloseCommand = new RelayCommand(OnClose);
            NewMySQL = new RelayCommand(OnNewMySQL);
            NewRedis = new RelayCommand(OnNewRedis);
            NewLogin = new RelayCommand(OnNewLogin);
            NewGame = new RelayCommand(OnNewGame);
            FindGatewayFile = new RelayCommand(OnFindGatewayFile);
            FindLoginFile = new RelayCommand(OnFindLoginFile);
            FindGameFile = new RelayCommand(OnFindGameFile);
            FindInternalFile = new RelayCommand(OnFindInternalFile);
            BuildCommand = new RelayCommand(OnBuild);
            RunCommand = new RelayCommand(OnRun);
        }

        private void OnRun(object obj)
        {

        }

        private void OnBuild(object obj)
        {
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void OnFindGatewayFile(object obj)
        {
            var dialog = new OpenFileDialog
            {
                DefaultExt = ".exe",
                Filter = "Executable Files (*.exe)|*.exe",
                Multiselect = false
            };
            if (dialog.ShowDialog() == false)
                return;

            GatewayFile = dialog.FileName;
        }

        private void OnFindLoginFile(object obj)
        {
            var dialog = new OpenFileDialog
            {
                DefaultExt = ".exe",
                Filter = "Executable Files (*.exe)|*.exe",
                Multiselect = false
            };
            if (dialog.ShowDialog() == false)
                return;

            LoginFile = dialog.FileName;
        }

        private void OnFindGameFile(object obj)
        {
            var dialog = new OpenFileDialog
            {
                DefaultExt = ".exe",
                Filter = "Executable Files (*.exe)|*.exe",
                Multiselect = false
            };
            if (dialog.ShowDialog() == false)
                return;

            GameFile = dialog.FileName;
        }

        private void OnFindInternalFile(object obj)
        {
            var dialog = new OpenFileDialog
            {
                DefaultExt = ".exe",
                Filter = "Executable Files (*.exe)|*.exe",
                Multiselect = false
            };
            if (dialog.ShowDialog() == false)
                return;

            InternalFile = dialog.FileName;
        }

        private void OnNewGame(object obj)
        {
            Game.Add(new GameConfig(new Model.GameConfig
            {
                ID = Game.Count,
                Port = 0
            }));
        }

        private void OnNewLogin(object obj)
        {
            Login.Add(new LoginConfig(new Model.LoginConfig
            {
                Name = string.Empty,
                Desc = string.Empty,
                Port = 0
            }));
        }

        private void Game_CollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case NotifyCollectionChangedAction.Add:
                    foreach (GameConfig item in e.NewItems)
                    {
                        Model.Game.Add(item.Model);
                    }
                    break;
                case NotifyCollectionChangedAction.Remove:
                    foreach (GameConfig item in e.OldItems)
                    {
                        Model.Game.Remove(item.Model);
                    }
                    break;
            }
        }

        private void Login_CollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case NotifyCollectionChangedAction.Add:
                    foreach (LoginConfig item in e.NewItems)
                    {
                        Model.Login.Add(item.Model);
                    }
                    break;
                case NotifyCollectionChangedAction.Remove:
                    foreach (LoginConfig item in e.OldItems)
                    {
                        Model.Login.Remove(item.Model);
                    }
                    break;
            }
        }

        private void Redis_CollectionChanged(object? sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
                    foreach (RedisConnection item in e.NewItems)
                    {
                        Model.Redis.Add(item.Model);
                    }
                    break;

                case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
                    foreach (RedisConnection item in e.OldItems)
                    {
                        Model.Redis.Remove(item.Model);
                    }
                    break;
            }
        }

        private void MySQL_CollectionChanged(object? sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
                    foreach (MySqlConnection item in e.NewItems)
                    {
                        Model.MySQL.Add(item.Model);
                    }
                    break;

                case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
                    foreach (MySqlConnection item in e.OldItems)
                    {
                        Model.MySQL.Remove(item.Model);
                    }
                    break;
            }
        }

        private void OnNewRedis(object obj)
        {
            Redis.Add(new RedisConnection(new Model.RedisConnection
            {
                IP = string.Empty,
                Port = 0
            }));
        }

        private void OnClose(object obj)
        {
            Model.Window.Close();
        }

        private void OnSetMaximize(object obj)
        {
            Model.Window.WindowState ^= System.Windows.WindowState.Maximized;
        }

        private void OnSetMinimize(object obj)
        {
            Model.Window.WindowState = System.Windows.WindowState.Minimized;
        }

        private void OnNewMySQL(object obj)
        {
            MySQL.Add(new MySqlConnection(new Model.MySqlConnection
            {
                IP = string.Empty,
                Name = MySQL.Count == 0 ? "GLOBAL" : "DATA",
                Port = 0
            }));
        }
    }
}
