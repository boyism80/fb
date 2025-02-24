using System.Collections.ObjectModel;
using System.Windows;

namespace runner;


public class MySqlConnection
{
    public string Name { get; set; } = "글로벌";
    public string IP { get; set; } = "127.0.0.1";
    public ushort Port { get; set; } = 3306;
}

public class RedisConnection
{
    public string IP { get; set; } = "127.0.0.1";
    public ushort Port { get; set; } = 3306;
}

public class RabbitMQConnection
{
    public string IP { get; set; } = "127.0.0.1";
    public ushort Port { get; set; } = 3306;
}

public class LoginConfig
{
    public string Name { get; set; }
    public string Desc { get; set; }
    public ushort Port { get; set; }
}

public class GameConfig
{
    public int ID { get; set; }
    public ushort Port { get; set; }
}

public class MainWindowViewModel
{
    public ObservableCollection<MySqlConnection> MySqlConnection { get; set; } = new ObservableCollection<MySqlConnection>();
    public ObservableCollection<RedisConnection> RedisConnection { get; set; } = new ObservableCollection<RedisConnection>();
    public ObservableCollection<RabbitMQConnection> RabbitMQConnection { get; set; } = new ObservableCollection<RabbitMQConnection>();
    public ObservableCollection<LoginConfig> LoginConfs { get; set; } = new ObservableCollection<LoginConfig>();
    public ObservableCollection<GameConfig> GameConfs { get; set; } = new ObservableCollection<GameConfig>();
}

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();

        var vm = new MainWindowViewModel();
        vm.MySqlConnection.Add(new MySqlConnection
        {
        });

        vm.RedisConnection.Add(new RedisConnection
        {
        });

        vm.RabbitMQConnection.Add(new RabbitMQConnection
        {
        });

        vm.LoginConfs.Add(new LoginConfig
        {
            Name = "연",
            Desc = "1996년 4월 서비스 시작",
            Port = 3002
        });

        vm.GameConfs.Add(new GameConfig
        {
            ID = 0,
            Port = 3003
        });
        DataContext = vm;
    }
}