using Newtonsoft.Json;
using System.Collections.ObjectModel;
using System.IO;
using System.Windows;

namespace Runner.Model
{
    public class MySqlConnection
    {
        public required string Name { get; set; }
        public required string IP { get; set; }
        public required ushort Port { get; set; }
    }

    public class RedisConnection
    {
        public required string IP { get; set; }
        public required ushort Port { get; set; }
    }

    public class RabbitMQConnection
    {
        public required string IP { get; set; }
        public required ushort Port { get; set; }
    }

    public class GatewayConfig
    {
        public required ushort Port { get; set; }
    }

    public class LoginConfig
    {
        public required string Name { get; set; }
        public required string Desc { get; set; }
        public required ushort Port { get; set; }
    }

    public class GameConfig
    {
        public required int ID { get; set; }
        public required ushort Port { get; set; }
    }

    public class InternalConfig
    {
        public required ushort Port { get; set; }
    }

    public class MainWindow
    {
        [JsonIgnore]
        public Window Window { get; private set; }

        public List<MySqlConnection> MySQL { get; set; } = new List<MySqlConnection>();
        public List<RedisConnection> Redis { get; set; } = new List<RedisConnection>();
        public RabbitMQConnection RabbitMq { get; set; } = new RabbitMQConnection
        {
            IP = string.Empty,
            Port = 0
        };
        public GatewayConfig Gateway { get; set; } = new GatewayConfig
        {
            Port = 0
        };
        public List<LoginConfig> Login { get; set; } = new List<LoginConfig>();
        public List<GameConfig> Game { get; set; } = new List<GameConfig>();
        public InternalConfig Internal { get; set; } = new InternalConfig
        {
            Port = 0
        };

        public MainWindow(Window window)
        {
            Window = window;
        }

        public void Save(string path)
        {
            File.WriteAllText(path, JsonConvert.SerializeObject(this, Formatting.Indented));
        }

        public static MainWindow Load(Window window, string path)
        {
            try
            {
                var model = JsonConvert.DeserializeObject<MainWindow>(File.ReadAllText(path));
                model.Window = window;
                return model;
            }
            catch (Exception e)
            {
                return null;
            }
        }
    }
}
