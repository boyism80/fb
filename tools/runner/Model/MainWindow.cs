using Newtonsoft.Json;
using System.IO;
using System.Windows;

namespace Runner.Model
{
    public class MySqlConnection
    {
        public required string Name { get; set; }
        public required string IP { get; set; }
        public required ushort Port { get; set; }
        public required string ID { get; set; }
        public required string PW { get; set; }
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
        public required string ID { get; set; }
        public required string PW { get; set; }
    }

    public class GatewaySetting
    {
        public required ushort Port { get; set; }
    }

    public class LoginSetting
    {
        public required string Name { get; set; }
        public required string Desc { get; set; }
        public required ushort Port { get; set; }
    }

    public class GameSetting
    {
        public required int ID { get; set; }
        public required ushort Port { get; set; }
    }

    public class InternalSetting
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
            IP = "127.0.0.1",
            Port = 5672,
            ID = "guest",
            PW = "guest",
        };
        public GatewaySetting Gateway { get; set; } = new GatewaySetting
        {
            Port = 3001
        };
        public List<LoginSetting> Login { get; set; } = new List<LoginSetting>();
        public List<GameSetting> Game { get; set; } = new List<GameSetting>();
        public InternalSetting Internal { get; set; } = new InternalSetting
        {
            Port = 3000
        };
        public DateTime LastBuildDate { get; set; }
        public string WorkingDirectory { get; set; }
        public string ExternalIP { get; set; }
        public string Agreement { get; set; }
        public uint InitMap { get; set; }
        public List<Point> InitPoints { get; set; } = new List<Point>();
        public uint BaseHP { get; set; } = 50;
        public uint AdditionalHP { get; set; } = 10;
        public uint BaseMP { get; set; } = 50;
        public uint AdditionalMP { get; set; } = 10;
        public bool AllowForeignName { get; set; } = false;
        public bool AdminMode { get; set; } = false;
        public byte MinIdLength { get; set; } = 2;
        public byte MaxIdLength { get; set; } = 12;
        public byte MinPwLength { get; set; } = 4;
        public byte MaxPwLength { get; set; } = 16;
        public uint SaveInterval { get; set; } = 600;

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
