using Microsoft.Win32;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Runner.Command;
using System.Buffers;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Net.Http;
using System.Net.Sockets;
using System.Text;
using System.Windows;
using System.Windows.Input;

namespace Runner.ViewModel
{
    public enum ServerType
    {
        Gateway,
        Game,
        Login,
        Internal,
        WriteBack
    }

    public class ServerProcess
    {
        public Process Process { get; set; }
        public string Name { get; set; }
        public string Output { get; set; }

        public override string ToString() => Name;
    }

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

        public string ID
        {
            get => Model.ID;
            set => Model.ID = value;
        }

        public string PW
        {
            get => Model.PW;
            set => Model.PW = value;
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

    public class RabbitMqConnection
    {
        public Model.RabbitMQConnection Model { get; private set; }
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

        public string ID
        {
            get => Model.ID;
            set => Model.ID = value;
        }

        public string PW
        {
            get => Model.PW;
            set => Model.PW = value;
        }

        public RabbitMqConnection(Model.RabbitMQConnection model)
        {
            Model = model;
        }
    }

    public class GatewaySetting
    {
        public Model.GatewaySetting Model { get; private set; }
        public ushort Port
        {
            get => Model.Port;
            set => Model.Port = value;
        }

        public GatewaySetting(Model.GatewaySetting model)
        {
            Model = model;
        }
    }

    public class LoginSetting
    {
        public Model.LoginSetting Model { get; private set; }
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

        public LoginSetting(Model.LoginSetting model)
        {
            Model = model;
        }
    }

    public class GameSetting
    {
        public Model.GameSetting Model { get; private set; }
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

        public GameSetting(Model.GameSetting model)
        {
            Model = model;
        }
    }

    public class MainWindow : INotifyPropertyChanged, IDisposable
    {
        public Model.MainWindow Model { get; private set; }

        public ObservableCollection<MySqlConnection> MySQL { get; set; } = new ObservableCollection<MySqlConnection>();
        public ObservableCollection<RedisConnection> Redis { get; set; } = new ObservableCollection<RedisConnection>();
        public RabbitMqConnection RabbitMq { get; set; }
        public ushort InternalPort
        {
            get => Model.Internal.Port;
            set => Model.Internal.Port = value;
        }
        public GatewaySetting Gateway { get; set; }
        public ObservableCollection<LoginSetting> Login { get; set; } = new ObservableCollection<LoginSetting>();
        public ObservableCollection<GameSetting> Game { get; set; } = new ObservableCollection<GameSetting>();
        public string Agreement
        {
            get => Model.Agreement;
            set => Model.Agreement = value;
        }
        public bool IsEnabled { get; set; } = true;
        public string WorkingDirectory
        {
            get => Model.WorkingDirectory;
            set => Model.WorkingDirectory = value;
        }
        public string ExternalIP
        {
            get => Model.ExternalIP;
            set => Model.ExternalIP = value;
        }

        private Process _buildProcess;
        public Process BuildProcess
        {
            get => _buildProcess;
            set
            {
                _buildProcess = value;
                PropertyChanged.Invoke(this, new PropertyChangedEventArgs(nameof(IsEnableBuild)));
            }
        }

        private bool _isConverting = false;
        public bool IsConverting
        {
            get => _isConverting;
            set
            {
                _isConverting = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsEnableEdit)));
            }
        }
        public bool IsEnableEdit
        {
            get
            {
                if (IsRunning)
                    return false;

                if (IsConverting)
                    return false;

                return true;
            }
        }

        public class ProcessGroup
        {
            public string Name => ToString();
            public ServerType Type { get; set; }
            public ObservableCollection<ServerProcess> Processes { get; set; } = new ObservableCollection<ServerProcess>();

            public override string ToString() => Type.ToString();
        }

        public ObservableCollection<ProcessGroup> Servers { get; set; } = new ObservableCollection<ProcessGroup>();

        private ServerProcess _serverProcess;
        public ServerProcess SelectedProcess
        {
            get => _serverProcess;
            set
            {
                _serverProcess = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MainText)));
            }
        }

        public string BuildLog { get; set; }
        public string MainText
        {
            get
            {
                if (SelectedProcess != null)
                    return SelectedProcess.Output;
                else
                    return BuildLog;
            }
        }
        public bool IsEnableBuild => BuildProcess == null;
        public bool IsEnableRun => BuildProcess == null;
        public bool IsRunning => Servers.SelectMany(x => x.Processes).Any();
        public bool IsEnableSettingTab => !IsRunning;
        public string RunButtonText
        {
            get
            {
                if (IsRunning)
                    return "종료";
                else
                    return "시작";
            }
        }

        public DateTime LastBuildDate
        {
            get => Model.LastBuildDate;
            set
            {
                Model.LastBuildDate = value;
                PropertyChanged.Invoke(this, new PropertyChangedEventArgs(nameof(LastBuildDateText)));
            }
        }

        public string LastBuildDateText
        {
            get
            {
                if (LastBuildDate == DateTime.MinValue)
                    return string.Empty;
                else
                    return $"마지막 빌드 : {LastBuildDate:yyyy-MM-dd HH:mm:ss}";
            }
        }

        public uint InitMap
        {
            get => Model.InitMap;
            set => Model.InitMap = value;
        }
        public ObservableCollection<Point> InitPoints { get; set; } = new ObservableCollection<Point>();

        public uint BaseHP
        {
            get => Model.BaseHP;
            set => Model.BaseHP = value;
        }

        public uint AdditionalHP
        {
            get => Model.AdditionalHP;
            set => Model.AdditionalHP = value;
        }

        public uint BaseMP
        {
            get => Model.BaseMP;
            set => Model.BaseMP = value;
        }

        public uint AdditionalMP
        {
            get => Model.AdditionalMP;
            set => Model.AdditionalMP = value;
        }

        public bool AllowForeignName
        {
            get => Model.AllowForeignName;
            set => Model.AllowForeignName = value;
        }

        public bool AdminMode
        {
            get => Model.AdminMode;
            set => Model.AdminMode = value;
        }

        public byte MinIdLength
        {
            get => Model.MinIdLength;
            set => Model.MinIdLength = value;
        }

        public byte MaxIdLength
        {
            get => Model.MaxIdLength;
            set => Model.MaxIdLength = value;
        }

        public byte MinPwLength
        {
            get => Model.MinPwLength;
            set => Model.MinPwLength = value;
        }

        public byte MaxPwLength
        {
            get => Model.MaxPwLength;
            set => Model.MaxPwLength = value;
        }

        public uint SaveInterval
        {
            get => Model.SaveInterval;
            set => Model.SaveInterval = value;
        }

        public ICommand SetMinimizeCommand { get; private set; }
        public ICommand SetMaximizeCommand { get; private set; }
        public ICommand CloseCommand { get; private set; }
        public ICommand NewMySQL { get; private set; }
        public ICommand NewRedis { get; private set; }
        public ICommand NewLogin { get; private set; }
        public ICommand NewGame { get; private set; }
        public ICommand FindWorkingDirectory { get; private set; }
        public ICommand PatchCommand { get; private set; }
        public ICommand RunCommand { get; private set; }
        public ICommand DeleteMySQL { get; private set; }
        public ICommand DeleteRedis { get; private set; }
        public ICommand DeleteLogin { get; private set; }
        public ICommand DeleteGame { get; private set; }
        public ICommand NewInitPoint { get; set; }
        public ICommand DeleteInitPoint { get; private set; }
        public ICommand UpdateMapFile { get; private set; }
        public ICommand UpdateResourceFile { get; private set; }
        public ICommand UpdateScript { get; private set; }

        public MainWindow(Model.MainWindow model)
        {
            Model = model;

            RabbitMq = new RabbitMqConnection(Model.RabbitMq);

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
                Login.Add(new LoginSetting(login));
            }
            Login.CollectionChanged += Login_CollectionChanged;

            foreach (var game in Model.Game)
            {
                Game.Add(new GameSetting(game));
            }
            Game.CollectionChanged += Game_CollectionChanged;

            foreach (var initPoint in Model.InitPoints)
            {
                InitPoints.Add(new Point(initPoint));
            }
            InitPoints.CollectionChanged += InitPoints_CollectionChanged;
            Servers.CollectionChanged += Servers_CollectionChanged;

            Gateway = new GatewaySetting(Model.Gateway);
            SetMinimizeCommand = new RelayCommand(OnSetMinimize);
            SetMaximizeCommand = new RelayCommand(OnSetMaximize);
            CloseCommand = new RelayCommand(OnClose);
            NewMySQL = new RelayCommand(OnNewMySQL);
            NewRedis = new RelayCommand(OnNewRedis);
            NewLogin = new RelayCommand(OnNewLogin);
            NewGame = new RelayCommand(OnNewGame);
            FindWorkingDirectory = new RelayCommand(OnFindWorkingDirectory);
            PatchCommand = new RelayCommand(OnPatch);
            RunCommand = new RelayCommand(OnRun);
            DeleteMySQL = new RelayCommand(OnDeleteMySQL);
            DeleteRedis = new RelayCommand(OnDeleteRedis);
            DeleteLogin = new RelayCommand(OnDeleteLogin);
            DeleteGame = new RelayCommand(OnDeleteGame);
            NewInitPoint = new RelayCommand(OnNewInitPoint);
            DeleteInitPoint = new RelayCommand(OnDeleteInitPoint);
            UpdateMapFile = new RelayCommand(OnUpdateMapFile);
            UpdateResourceFile = new RelayCommand(OnUpdateResourceFile);
            UpdateScript = new RelayCommand(OnUpdateScript);
        }

        private void Servers_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsRunning)));
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsEnableSettingTab)));
        }

        private void InitPoints_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case NotifyCollectionChangedAction.Add:
                    foreach (Point item in e.NewItems)
                    {
                        Model.InitPoints.Add(item.Model);
                    }
                    break;
                case NotifyCollectionChangedAction.Remove:
                    foreach (Point item in e.OldItems)
                    {
                        Model.InitPoints.Remove(item.Model);
                    }
                    break;
            }
        }

        private void OnUpdateMapFile(object obj)
        {
            try
            {
                var path = Path.Combine(WorkingDirectory, "resources", "maps", "maps.zip");
                if (File.Exists(path) == false)
                    throw new InvalidOperationException($"{path} 파일을 찾을 수 없습니다.");

                var output = Path.Combine(WorkingDirectory, "build", "dist", "maps");
                System.IO.Compression.ZipFile.ExtractToDirectory(path, output, true);
                MessageBox.Show("맵 파일을 업데이트 했습니다.", "완료");
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }

        private void OnUpdateResourceFile(object obj)
        {
            if (IsConverting)
                return;

            if (string.IsNullOrEmpty(WorkingDirectory) || Directory.Exists(WorkingDirectory) == false)
                return;

            var p = new Process
            {
                EnableRaisingEvents = true,
                StartInfo = new ProcessStartInfo
                {
                    WindowStyle = ProcessWindowStyle.Hidden,
                    WorkingDirectory = WorkingDirectory,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    StandardOutputEncoding = Encoding.UTF8,
                    StandardErrorEncoding = Encoding.UTF8,
                    FileName = "cmd.exe",
                    Arguments = @"/C pushd tools & call update-data.bat true & popd & robocopy /NP /NFL game\\json\\ build\\dist\\json\\ & robocopy /NP /NFL internal\\json\\ build\\dist\\internal\\json\\"
                }
            };

            SelectedProcess = null;
            BuildLog = string.Empty;
            p.OutputDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                BuildLog += (e.Data + Environment.NewLine);
            };
            p.ErrorDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                BuildLog += (e.Data + Environment.NewLine);
            };
            p.Start();
            p.BeginOutputReadLine();
            p.BeginErrorReadLine();
            p.Exited += (sender, e) =>
            {
                IsConverting = false;
            };

            IsConverting = true;
        }

        private void OnUpdateScript(object obj)
        {
            try
            {
                var path = Path.Combine(WorkingDirectory, "game", "scripts");
                if (Directory.Exists(path) == false)
                    throw new InvalidOperationException($"{path} 경로를 찾을 수 없습니다.");

                var output = Path.Combine(WorkingDirectory, "build", "dist", "scripts");
                CopyFolderContents(path, output);
                MessageBox.Show("스크립트 파일을 업데이트 했습니다.", "완료");
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }

        private void OnDeleteInitPoint(object obj)
        {
            InitPoints.Remove(obj as Point);
        }

        private void OnNewInitPoint(object obj)
        {
            InitPoints.Add(new Point(new Runner.Model.Point
            {
                X = 0,
                Y = 0
            }));
        }

        private void OnDeleteMySQL(object obj)
        {
            MySQL.Remove(obj as MySqlConnection);
        }

        private void OnDeleteRedis(object obj)
        {
            Redis.Remove(obj as RedisConnection);
        }

        private void OnDeleteLogin(object obj)
        {
            Login.Remove(obj as LoginSetting);
        }

        private void OnDeleteGame(object obj)
        {
            Game.Remove(obj as GameSetting);
        }

        private void OnRun(object obj)
        {
            try
            {
                if (IsRunning)
                {
                    KillProcesses();
                    return;
                }

                if (Login.Count == 0)
                    throw new InvalidOperationException("로그인 서버를 하나 이상 추가해야 합니다.");

                if (Game.Count == 0)
                    throw new InvalidOperationException("게임 서버를 하나 이상 추가해야 합니다.");

                if (InternalPort == 0)
                    throw new InvalidOperationException("Internal 서버 포트가 설정되지 않았습니다.");

                if (string.IsNullOrEmpty(ExternalIP))
                    throw new InvalidOperationException("IP가 설정되지 않았습니다.");

                if (string.IsNullOrEmpty(RabbitMq.IP))
                    throw new InvalidOperationException("RabbitMQ IP가 설정되지 않았습니다.");

                if (RabbitMq.Port == 0)
                    throw new InvalidOperationException("RabbitMQ Port가 설정되지 않았습니다.");

                if (string.IsNullOrEmpty(RabbitMq.ID))
                    throw new InvalidOperationException("RabbitMQ 아이디가 설정되지 않았습니다.");

                if (string.IsNullOrEmpty(RabbitMq.PW))
                    throw new InvalidOperationException("RabbitMQ 암호가 설정되지 않았습니다.");

                if (BaseHP == 0)
                    throw new InvalidOperationException("기본 체력은 1 이상이어야 합니다.");

                if (BaseMP == 0)
                    throw new InvalidOperationException("기본 마력은 1 이상이어야 합니다.");

                if (MinIdLength == 0)
                    throw new InvalidOperationException("최소 아이디 길이는 1 이상이어야 합니다.");

                if (MaxIdLength < MinIdLength)
                    throw new InvalidOperationException("최대 아이디 길이는 최소 아이디 길이보다 길어야 합니다.");

                if (MinPwLength == 0)
                    throw new InvalidOperationException("최소 암호 길이는 1 이상이어야 합니다.");

                if (MaxPwLength < MinPwLength)
                    throw new InvalidOperationException("최대 암호 길이는 최소 암호 길이보다 길어야 합니다.");

                if (SaveInterval == 0)
                    throw new InvalidOperationException("서버 저장 주기는 1초 이상이어야 합니다.");

                foreach (var mysql in MySQL)
                {
                    if (Ping(mysql.IP, mysql.Port) == false)
                        throw new InvalidOperationException($"MySQL {mysql.IP}:{mysql.Port}가 열리지 않았습니다.");
                }

                foreach (var redis in Redis)
                {
                    if (Ping(redis.IP, redis.Port) == false)
                        throw new InvalidOperationException($"Redis {redis.IP}:{redis.Port}가 열리지 않았습니다.");
                }

                if (Ping(RabbitMq.IP, RabbitMq.Port) == false)
                    throw new InvalidOperationException($"RabbitMQ {RabbitMq.IP}:{RabbitMq.Port}가 열리지 않았습니다.");

                var confDir = Path.Combine([WorkingDirectory, "build", "dist", "config"]);
                if (Directory.Exists(confDir) == false)
                    Directory.CreateDirectory(confDir);

                var loginDir = Path.Combine(confDir, "login");
                var gatewayDir = Path.Combine(confDir, "gateway");
                var gameDir = Path.Combine(confDir, "game");

                if (Directory.Exists(loginDir) == false)
                    Directory.CreateDirectory(loginDir);
                if (Directory.Exists(gatewayDir) == false)
                    Directory.CreateDirectory(gatewayDir);
                if (Directory.Exists(gameDir) == false)
                    Directory.CreateDirectory(gameDir);

                for (int i = 0; i < Login.Count; i++)
                {
                    var setting = Login[i];
                    if (string.IsNullOrEmpty(setting.Name))
                        throw new InvalidOperationException($"{i + 1}번째 로그인 서버의 이름이 설정되지 않았습니다.");

                    if (string.IsNullOrEmpty(setting.Desc))
                        throw new InvalidOperationException($"{i + 1}번째 로그인 서버의 설명이 설정되지 않았습니다.");

                    if (setting.Port == 0)
                        throw new InvalidOperationException($"{i + 1}번째 로그인 서버의 포트가 설정되지 않았습니다.");

                    var conf = new JObject();
                    conf["id"] = i;
                    conf["name"] = $"login-{i}";
                    conf["world"] = 1;
                    conf["ip"] = ExternalIP;
                    conf["port"] = setting.Port;
                    conf["transfer delay"] = 0;
                    conf["allow_foreign_name"] = AllowForeignName;
                    conf["agreement"] = Agreement;
                    conf["admin_mode"] = AdminMode;
                    conf["thread"] = JObject.FromObject(new
                    {
                        logic = 12,
                        io = 12
                    });
                    var amqp = new JObject();
                    amqp["internal"] = JObject.FromObject(new { ip = RabbitMq.IP, port = RabbitMq.Port, uid = RabbitMq.ID, pwd = RabbitMq.PW });
                    amqp["log"] = JObject.FromObject(new { ip = RabbitMq.IP, port = RabbitMq.Port, uid = RabbitMq.ID, pwd = RabbitMq.PW });
                    conf["amqp"] = amqp;
                    conf["internal"] = JObject.FromObject(new
                    {
                        ip = "127.0.0.1",
                        port = InternalPort
                    });
                    conf["log"] = JObject.FromObject(new
                    {
                        ip = ExternalIP,
                        port = (ushort)3005,
                        level = new JArray("debug", "info", "warn", "fatal")
                    });
                    conf["init"] = new JObject();
                    conf["init"]["map"] = InitMap;
                    conf["init"]["position"] = new JArray();
                    foreach (var point in InitPoints)
                    {
                        (conf["init"]["position"] as JArray).Add(JObject.FromObject(new
                        {
                            x = point.X,
                            y = point.Y
                        }));
                    }
                    conf["init"]["hp"] = new JObject();
                    conf["init"]["hp"]["base"] = BaseHP;
                    conf["init"]["hp"]["range"] = AdditionalHP;
                    conf["init"]["mp"] = new JObject();
                    conf["init"]["mp"]["base"] = BaseMP;
                    conf["init"]["mp"]["range"] = AdditionalMP;
                    conf["name_size"] = JObject.FromObject(new
                    {
                        min = MinIdLength,
                        max = MaxIdLength
                    });
                    conf["pw_size"] = JObject.FromObject(new
                    {
                        min = MinPwLength,
                        max = MaxPwLength
                    });
                    File.WriteAllText(Path.Combine([loginDir, $"config_login_{i}.json"]), conf.ToString(Formatting.Indented));
                }

                for (int i = 0; i < Game.Count; i++)
                {
                    var setting = Game[i];
                    if (setting.Port == 0)
                        throw new InvalidOperationException($"{i + 1}번째 게임 서버의 포트가 설정되지 않았습니다.");

                    var conf = new JObject();
                    conf["id"] = setting.ID;
                    conf["name"] = $"game-{setting.ID}";
                    conf["world"] = 1;
                    conf["delay"] = 5;
                    conf["ip"] = ExternalIP;
                    conf["port"] = setting.Port;
                    conf["thread"] = JObject.FromObject(new
                    {
                        logic = 12,
                        io = 12
                    });
                    conf["save"] = SaveInterval;
                    conf["internal"] = JObject.FromObject(new
                    {
                        ip = "127.0.0.1",
                        port = InternalPort
                    });
                    conf["marketplace"] = JObject.FromObject(new
                    {
                        ip = ExternalIP,
                        port = (ushort)3010
                    });
                    conf["login"] = JObject.FromObject(new
                    {
                        ip = ExternalIP,
                        port = Login[0].Port
                    });
                    var gameAmqp = new JObject();
                    gameAmqp["internal"] = JObject.FromObject(new { ip = RabbitMq.IP, port = RabbitMq.Port, uid = RabbitMq.ID, pwd = RabbitMq.PW });
                    gameAmqp["log"] = JObject.FromObject(new { ip = RabbitMq.IP, port = RabbitMq.Port, uid = RabbitMq.ID, pwd = RabbitMq.PW });
                    conf["amqp"] = gameAmqp;
                    conf["log"] = JObject.FromObject(new
                    {
                        ip = ExternalIP,
                        port = (ushort)3005,
                        level = new JArray("debug", "info", "warn", "fatal")
                    });
                    conf["lazy_load_maps"] = true;
                    conf["exp_multiplier"] = 100.0;
                    conf["drop_rate_multiplier"] = 100.0;
                    conf["http"] = JObject.FromObject(new
                    {
                        max_concurrent = 128
                    });
                    File.WriteAllText(Path.Combine([gameDir, $"config_game_{setting.ID}.json"]), conf.ToString(Formatting.Indented));
                }

                if (Gateway.Port == 0)
                    throw new InvalidOperationException("게이트웨이 서버의 포트가 설정되지 않았습니다.");

                var gatewayConf = new JObject();
                gatewayConf["id"] = 0;
                gatewayConf["ip"] = ExternalIP;
                gatewayConf["name"] = "gateway";
                gatewayConf["port"] = Gateway.Port;
                gatewayConf["thread"] = JObject.FromObject(new
                {
                    logic = 12,
                    io = 12
                });
                gatewayConf["log"] = JObject.FromObject(new
                {
                    ip = ExternalIP,
                    port = (ushort)3005,
                    level = new JArray("debug", "info", "warn", "fatal")
                });
                gatewayConf["entrypoints"] = new JArray();
                for (int i = 0; i < Login.Count; i++)
                {
                    var setting = Login[i];
                    (gatewayConf["entrypoints"] as JArray).Add(JObject.FromObject(new
                    {
                        name = setting.Name,
                        desc = setting.Desc,
                        ip = ExternalIP,
                        port = setting.Port
                    }));
                }
                gatewayConf["internal"] = JObject.FromObject(new
                {
                    ip = "127.0.0.1",
                    port = InternalPort
                });
                var gatewayAmqp = new JObject();
                gatewayAmqp["internal"] = JObject.FromObject(new { ip = RabbitMq.IP, port = RabbitMq.Port, uid = RabbitMq.ID, pwd = RabbitMq.PW });
                gatewayAmqp["log"] = JObject.FromObject(new { ip = RabbitMq.IP, port = RabbitMq.Port, uid = RabbitMq.ID, pwd = RabbitMq.PW });
                gatewayConf["amqp"] = gatewayAmqp;

                File.WriteAllText(Path.Combine([gatewayDir, $"config_gateway.json"]), gatewayConf.ToString(Formatting.Indented));

                var internalConf = new JObject();
                internalConf["Logging"] = new JObject();
                internalConf["Logging"]["LogLevel"] = new JObject();
                internalConf["Logging"]["LogLevel"]["Default"] = "Information";
                internalConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";

                var internalMySql = new JObject();
                if (MySQL.Count > 0)
                {
                    var first = MySQL[0];
                    internalMySql["unified"] = $"Server={first.IP};Port={first.Port};User ID={first.ID}; Password={first.PW}; Database=fb";
                }
                var internalMySqlWorlds = new JObject();
                var world1MySql = new JObject();
                var dataConnStrings = new JArray();
                foreach (var db in MySQL)
                    dataConnStrings.Add($"Server={db.IP};Port={db.Port};User ID={db.ID}; Password={db.PW}; Database=fb");
                if (MySQL.Count > 0)
                {
                    var first = MySQL[0];
                    world1MySql["global"] = $"Server={first.IP};Port={first.Port};User ID={first.ID}; Password={first.PW}; Database=fb";
                    if (dataConnStrings.Count > 0)
                        world1MySql["data"] = dataConnStrings;
                }
                internalMySqlWorlds["1"] = world1MySql;
                internalMySql["worlds"] = internalMySqlWorlds;
                internalConf["ConnectionStrings"] = new JObject();
                internalConf["ConnectionStrings"]["MySql"] = internalMySql;

                var internalRedis = new JObject();
                if (Redis.Count > 0)
                {
                    var first = Redis[0];
                    internalRedis["unified"] = JObject.FromObject(new { Host = first.IP, Port = first.Port });
                }
                var internalRedisWorlds = new JObject();
                var world1Redis = new JObject();
                var redisDataArray = new JArray();
                foreach (var r in Redis)
                    redisDataArray.Add(JObject.FromObject(new { Host = r.IP, Port = r.Port }));
                if (Redis.Count > 0)
                {
                    var first = Redis[0];
                    world1Redis["global"] = JObject.FromObject(new { Host = first.IP, Port = first.Port });
                    if (redisDataArray.Count > 0)
                        world1Redis["data"] = redisDataArray;
                }
                internalRedisWorlds["1"] = world1Redis;
                internalRedis["worlds"] = internalRedisWorlds;
                internalConf["Redis"] = internalRedis;

                var internalRabbit = new JObject();
                internalRabbit["Internal"] = JObject.FromObject(new { Host = RabbitMq.IP, Port = RabbitMq.Port, Uid = RabbitMq.ID, Pwd = RabbitMq.PW });
                internalRabbit["Log"] = JObject.FromObject(new { Host = RabbitMq.IP, Port = RabbitMq.Port, Uid = RabbitMq.ID, Pwd = RabbitMq.PW });
                internalConf["RabbitMQ"] = internalRabbit;
                internalConf["Log"] = JObject.FromObject(new { Enabled = true, ServerId = "0", ServerName = "internal" });
                File.WriteAllText(Path.Combine([WorkingDirectory, "build", "dist", "internal", "appsettings.internal.json"]), internalConf.ToString(Formatting.Indented));

                var wbConf = new JObject();
                wbConf["World"] = 1;
                wbConf["Logging"] = new JObject();
                wbConf["Logging"]["LogLevel"] = new JObject();
                wbConf["Logging"]["LogLevel"]["Default"] = "Information";
                wbConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";

                var wbMySql = new JObject();
                var wbMySqlWorlds = new JObject();
                var wbWorld1MySql = new JObject();
                var wbDataConnStrings = new JArray();
                foreach (var db in MySQL)
                    wbDataConnStrings.Add($"Server={db.IP};Port={db.Port};User ID={db.ID}; Password={db.PW}; Database=fb");
                if (MySQL.Count > 0)
                {
                    var first = MySQL[0];
                    wbWorld1MySql["global"] = $"Server={first.IP};Port={first.Port};User ID={first.ID}; Password={first.PW}; Database=fb";
                    if (wbDataConnStrings.Count > 0)
                        wbWorld1MySql["data"] = wbDataConnStrings;
                }
                wbMySqlWorlds["1"] = wbWorld1MySql;
                wbMySql["worlds"] = wbMySqlWorlds;
                wbConf["ConnectionStrings"] = new JObject();
                wbConf["ConnectionStrings"]["MySql"] = wbMySql;

                var wbRedis = new JObject();
                var wbRedisWorlds = new JObject();
                var wbWorld1Redis = new JObject();
                var wbRedisDataArray = new JArray();
                foreach (var r in Redis)
                    wbRedisDataArray.Add(JObject.FromObject(new { Host = r.IP, Port = r.Port }));
                if (Redis.Count > 0)
                {
                    var first = Redis[0];
                    wbWorld1Redis["global"] = JObject.FromObject(new { Host = first.IP, Port = first.Port });
                    if (wbRedisDataArray.Count > 0)
                        wbWorld1Redis["data"] = wbRedisDataArray;
                }
                wbRedisWorlds["1"] = wbWorld1Redis;
                wbRedis["worlds"] = wbRedisWorlds;
                wbConf["Redis"] = wbRedis;

                var wbRabbit = new JObject();
                wbRabbit["Internal"] = JObject.FromObject(new { Host = RabbitMq.IP, Port = RabbitMq.Port, Uid = RabbitMq.ID, Pwd = RabbitMq.PW });
                wbRabbit["Log"] = JObject.FromObject(new { Host = RabbitMq.IP, Port = RabbitMq.Port, Uid = RabbitMq.ID, Pwd = RabbitMq.PW });
                wbConf["RabbitMQ"] = wbRabbit;
                wbConf["Log"] = JObject.FromObject(new { Enabled = true, ServerId = "0", ServerName = "write-back" });
                File.WriteAllText(Path.Combine([WorkingDirectory, "build", "dist", "write-back", "appsettings.write-back.json"]), wbConf.ToString(Formatting.Indented));

                var gateway = new ProcessGroup { Type = ServerType.Gateway };
                gateway.Processes.Add(ExecCPP("gateway.exe", "gateway", "config_gateway"));
                Servers.Add(gateway);

                var login = new ProcessGroup { Type = ServerType.Login };
                for (int i = 0; i < Login.Count; i++)
                {
                    login.Processes.Add(ExecCPP("login.exe", $"login-{i}", $"config_login_{i}"));
                }
                Servers.Add(login);

                var game = new ProcessGroup { Type = ServerType.Game };
                for (int i = 0; i < Game.Count; i++)
                {
                    var setting = Game[i];
                    game.Processes.Add(ExecCPP("game.exe", $"game-{i}", $"config_game_{setting.ID}"));
                }
                Servers.Add(game);

                var inter = new ProcessGroup { Type = ServerType.Internal };
                inter.Processes.Add(ExecDotNet("internal", InternalPort));
                Servers.Add(inter);

                var wb = new ProcessGroup { Type = ServerType.WriteBack };
                wb.Processes.Add(ExecDotNet("write-back"));
                Servers.Add(wb);
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "오류", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            finally
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsEnableEdit)));
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RunButtonText)));
            }
        }

        private ServerProcess ExecCPP(string file, string env, string configFileName)
        {
            var serverType = env.Split('-')[0]; // gateway, login, game
            var process = new Process
            {
                EnableRaisingEvents = true,
                StartInfo = new ProcessStartInfo
                {
                    WindowStyle = ProcessWindowStyle.Hidden,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    RedirectStandardInput = true,
                    UseShellExecute = false,
                    WorkingDirectory = Path.Combine([WorkingDirectory, "build", "dist"]),
                    FileName = Path.Combine(WorkingDirectory, "build", "dist", file),
                    Arguments = $"-c config/{serverType}/{configFileName}.json"
                }
            };

            if (Path.Exists(process.StartInfo.FileName) == false)
                throw new InvalidOperationException($"{Path.GetFileName(process.StartInfo.FileName)} 파일이 없습니다. 빌드가 되어있는지 확인하세요.");

            var sp = new ServerProcess
            {
                Name = env,
                Process = process
            };

            process.OutputDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                sp.Output += (e.Data + Environment.NewLine);
                if (SelectedProcess == sp)
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MainText)));
            };
            process.ErrorDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                sp.Output += (e.Data + Environment.NewLine);
                if (SelectedProcess == sp)
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MainText)));
            };
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.Exited += (sender, e) =>
            {
                sp.Process = null;
            };

            return sp;
        }

        private ServerProcess ExecDotNet(string name, ushort? port = null)
        {
            var process = new Process
            {
                EnableRaisingEvents = true,
                StartInfo = new ProcessStartInfo
                {
                    WindowStyle = ProcessWindowStyle.Hidden,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    RedirectStandardInput = true,
                    UseShellExecute = false,
                    WorkingDirectory = Path.Combine([WorkingDirectory, "build", "dist", name]),
                    FileName = Path.Combine(WorkingDirectory, "build", "dist", name, $"{name}.exe"),
                    EnvironmentVariables =
                    {
                        ["ASPNETCORE_ENVIRONMENT"] = name,
                        ["ASPNETCORE_HTTP_PORTS"] = port?.ToString(),
                    }
                }
            };

            if (Path.Exists(process.StartInfo.FileName) == false)
                throw new InvalidOperationException($"{Path.GetFileName(process.StartInfo.FileName)} 파일이 없습니다. 빌드가 되어있는지 확인하세요.");

            var sp = new ServerProcess
            {
                Process = process,
                Name = name
            };

            process.OutputDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                sp.Output += (e.Data + Environment.NewLine);
                if (SelectedProcess == sp)
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MainText)));
            };
            process.ErrorDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                sp.Output += (e.Data + Environment.NewLine);
                if (SelectedProcess == sp)
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MainText)));
            };
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.Exited += (sender, e) =>
            {
                sp.Process = null;
            };

            return sp;
        }

        private static async Task DownloadFileAsync(string url, string destinationPath, Action<long, long> progressCallback)
        {
            using (var httpClient = new HttpClient())
            using (var response = await httpClient.GetAsync(url, HttpCompletionOption.ResponseHeadersRead))
            {
                response.EnsureSuccessStatusCode();

                var totalBytes = response.Content.Headers.ContentLength ?? -1L;
                var bufferSize = 128 * 1024;
                using (var contentStream = await response.Content.ReadAsStreamAsync())
                using (var fileStream = new FileStream(
                           destinationPath,
                           FileMode.Create,
                           FileAccess.Write,
                           FileShare.None,
                           bufferSize: bufferSize,
                           useAsync: true))
                {
                    var pool = ArrayPool<byte>.Shared;
                    var buffer = pool.Rent(bufferSize);

                    long totalRead = 0;
                    int bytesRead;
                    long lastLoggedMb = 0;

                    while ((bytesRead = await contentStream.ReadAsync(buffer, 0, bufferSize)) > 0)
                    {
                        await fileStream.WriteAsync(buffer, 0, bytesRead);
                        totalRead += bytesRead;

                        var currentMb = totalRead / (1024 * 1024);
                        if (currentMb > lastLoggedMb)
                        {
                            lastLoggedMb = currentMb;
                            progressCallback(currentMb, totalBytes / (1024 * 1024));
                        }
                    }

                    pool.Return(buffer);
                }

                progressCallback(-1, totalBytes / (1024 * 1024));
            }
        }

        private static async Task ExtractZipAsync(string zipPath, string destDir, Action<int, int, string> progressCallback)
        {
            if (!Directory.Exists(destDir))
                Directory.CreateDirectory(destDir);

            List<string> entryNames;
            using (var archive = ZipFile.OpenRead(zipPath))
            {
                entryNames = archive.Entries
                    .Where(e => !string.IsNullOrEmpty(e.Name))
                    .Select(e => e.FullName)
                    .ToList();
            }

            int totalFiles = entryNames.Count;
            int extractedCount = 0;

            await Task.Run(() =>
            {
                Parallel.ForEach(
                    entryNames,
                    new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount },
                    entryName =>
                    {
                        using (var fs = new FileStream(zipPath, FileMode.Open, FileAccess.Read, FileShare.Read))
                        using (var localArchive = new ZipArchive(fs, ZipArchiveMode.Read, leaveOpen: false))
                        {
                            var entry = localArchive.GetEntry(entryName);
                            var destinationPath = Path.Combine(destDir, entry.FullName);
                            Directory.CreateDirectory(Path.GetDirectoryName(destinationPath));

                            using (var entryStream = entry.Open())
                            using (var fileStream = new FileStream(
                                       destinationPath,
                                       FileMode.Create,
                                       FileAccess.Write,
                                       FileShare.None,
                                       bufferSize: 65536,
                                       useAsync: false))
                            {
                                entryStream.CopyTo(fileStream);
                            }
                        }

                        int count = Interlocked.Increment(ref extractedCount);
                        progressCallback(count, totalFiles, entryName);
                    });
            });
        }

        private static void ParallelDirectoryCopy(string sourceDir, string destDir)
        {
            if (!Directory.Exists(sourceDir))
                return;

            foreach (var dirPath in Directory.EnumerateDirectories(sourceDir, "*", SearchOption.AllDirectories))
            {
                var relativePath = Path.GetRelativePath(sourceDir, dirPath);
                var targetPath = Path.Combine(destDir, relativePath);
                Directory.CreateDirectory(targetPath);
            }

            var allFiles = Directory.EnumerateFiles(sourceDir, "*", SearchOption.AllDirectories);
            Parallel.ForEach(
                allFiles,
                new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount },
                filePath =>
                {
                    var relativeFile = Path.GetRelativePath(sourceDir, filePath);
                    var destFile = Path.Combine(destDir, relativeFile);
                    Directory.CreateDirectory(Path.GetDirectoryName(destFile));
                    File.Copy(filePath, destFile, overwrite: true);
                });
        }

        private void OnPatch(object obj)
        {
            if (IsConverting)
                return;

            if (string.IsNullOrEmpty(WorkingDirectory) || !Directory.Exists(WorkingDirectory))
                return;

            IsConverting = true;
            BuildLog = string.Empty;

            var zipUrl = "http://cshyeon.com:8080/fb/dist.zip";
            var buildDir = Path.Combine(WorkingDirectory, "build");
            var distDir = Path.Combine(buildDir, "dist");
            var zipPath = Path.Combine(buildDir, "dist.zip");

            Task.Run(async () =>
            {
                try
                {
                    if (!Directory.Exists(buildDir))
                        Directory.CreateDirectory(buildDir);

                    if (Directory.Exists(distDir))
                    {
                        Directory.Delete(distDir, recursive: true);
                        await Application.Current.Dispatcher.InvokeAsync(() =>
                        {
                            BuildLog += $"Deleted existing directory: {distDir}{Environment.NewLine}";
                        });
                    }


                    await DownloadFileAsync(zipUrl, zipPath, (currentMb, totalMb) =>
                    {
                        Application.Current.Dispatcher.Invoke(() =>
                        {
                            if (currentMb < 0)
                                BuildLog += $"Download completed: {zipPath}{Environment.NewLine}";
                            else
                                BuildLog += totalMb > 0
                                    ? $"Downloading: {currentMb} MB / {totalMb} MB{Environment.NewLine}"
                                    : $"Downloading: {currentMb} MB{Environment.NewLine}";
                        });
                    });

                    await Application.Current.Dispatcher.InvokeAsync(() =>
                    {
                        BuildLog += $"Extracting ZIP to: {distDir}{Environment.NewLine}";
                    });

                    await ExtractZipAsync(zipPath, distDir, (count, total, entryName) =>
                    {
                        Application.Current.Dispatcher.Invoke(() =>
                        {
                            BuildLog += $"Extracted {count}/{total}: {entryName}{Environment.NewLine}";
                        });
                    });

                    if (File.Exists(zipPath))
                    {
                        File.Delete(zipPath);
                        await Application.Current.Dispatcher.InvokeAsync(() =>
                        {
                            BuildLog += $"Deleted ZIP file: {zipPath}{Environment.NewLine}";
                        });
                    }

                    var subDirs = new[] { "json", "maps", "scripts" };
                    foreach (var sub in subDirs)
                    {
                        var sourceDir = Path.Combine(WorkingDirectory, "game", sub);
                        var targetDir = Path.Combine(distDir, sub);

                        if (Directory.Exists(targetDir))
                        {
                            Directory.Delete(targetDir, recursive: true);
                            await Application.Current.Dispatcher.InvokeAsync(() =>
                            {
                                BuildLog += $"Deleted: {targetDir}{Environment.NewLine}";
                            });
                        }

                        if (Directory.Exists(sourceDir))
                        {
                            await Task.Run(() =>
                            {
                                ParallelDirectoryCopy(sourceDir, targetDir);
                            });
                            await Application.Current.Dispatcher.InvokeAsync(() =>
                            {
                                BuildLog += $"Copied (parallel): {sourceDir} → {targetDir}{Environment.NewLine}";
                            });
                        }
                        else
                        {
                            await Application.Current.Dispatcher.InvokeAsync(() =>
                            {
                                BuildLog += $"WARNING: Source {sub.ToUpper()} not found: {sourceDir}{Environment.NewLine}";
                            });
                        }
                    }

                    await Application.Current.Dispatcher.InvokeAsync(() =>
                    {
                        LastBuildDate = DateTime.Now;
                        BuildLog += $"Patch + post-copy completed at {LastBuildDate}{Environment.NewLine}";
                        MessageBox.Show("패치가 완료되었습니다.", "알림", MessageBoxButton.OK, MessageBoxImage.Information);
                    });
                }
                catch (Exception ex)
                {
                    await Application.Current.Dispatcher.InvokeAsync(() =>
                    {
                        BuildLog += $"Error during patch: {ex}{Environment.NewLine}";
                        MessageBox.Show("에러가 발생했습니다.", "알림", MessageBoxButton.OK, MessageBoxImage.Information);
                    });
                }
                finally
                {
                    await Application.Current.Dispatcher.InvokeAsync(() =>
                    {
                        IsConverting = false;
                    });
                }
            });
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void OnFindWorkingDirectory(object obj)
        {
            var dialog = new OpenFolderDialog
            {
                Multiselect = false
            };
            if (dialog.ShowDialog() == false)
                return;

            WorkingDirectory = dialog.FolderName;
        }

        private void OnNewGame(object obj)
        {
            Game.Add(new GameSetting(new Model.GameSetting
            {
                ID = Game.Count,
                Port = 0
            }));
        }

        private void OnNewLogin(object obj)
        {
            Login.Add(new LoginSetting(new Model.LoginSetting
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
                    foreach (GameSetting item in e.NewItems)
                    {
                        Model.Game.Add(item.Model);
                    }
                    break;
                case NotifyCollectionChangedAction.Remove:
                    foreach (GameSetting item in e.OldItems)
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
                    foreach (LoginSetting item in e.NewItems)
                    {
                        Model.Login.Add(item.Model);
                    }
                    break;
                case NotifyCollectionChangedAction.Remove:
                    foreach (LoginSetting item in e.OldItems)
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
                IP = "127.0.0.1",
                Port = 6379
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
                IP = "127.0.0.1",
                Name = MySQL.Count == 0 ? "GLOBAL" : "DATA",
                Port = 3306,
                ID = "root",
                PW = "admin"
            }));
        }

        private void KillProcesses()
        {
            if (BuildProcess != null)
            {
                BuildProcess.Kill();
                BuildProcess = null;
            }

            foreach (var srv in Servers)
            {
                foreach (var process in srv.Processes)
                {
                    process.Process?.Kill();
                }
            }

            Servers.Clear();
            SelectedProcess = null;
        }

        public void Dispose()
        {
            KillProcesses();
        }

        // https://www.codeproject.com/Tips/278248/Recursively-Copy-folder-contents-to-another-in-Csh
        private bool CopyFolderContents(string SourcePath, string DestinationPath)
        {
            SourcePath = SourcePath.EndsWith(@"\") ? SourcePath : SourcePath + @"\";
            DestinationPath = DestinationPath.EndsWith(@"\") ? DestinationPath : DestinationPath + @"\";

            try
            {
                if (Directory.Exists(SourcePath))
                {
                    if (Directory.Exists(DestinationPath) == false)
                    {
                        Directory.CreateDirectory(DestinationPath);
                    }

                    foreach (string files in Directory.GetFiles(SourcePath))
                    {
                        FileInfo fileInfo = new FileInfo(files);
                        fileInfo.CopyTo(string.Format(@"{0}\{1}", DestinationPath, fileInfo.Name), true);
                    }

                    foreach (string drs in Directory.GetDirectories(SourcePath))
                    {
                        DirectoryInfo directoryInfo = new DirectoryInfo(drs);
                        if (CopyFolderContents(drs, DestinationPath + directoryInfo.Name) == false)
                        {
                            return false;
                        }
                    }
                }
                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        private static bool Ping(string ip, ushort port)
        {
            using (TcpClient tcpClient = new TcpClient())
            {
                try
                {
                    tcpClient.Connect(ip, port);
                    return true;
                }
                catch (Exception)
                {
                    return false;
                }
            }
        }
    }
}
