using Microsoft.Win32;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Runner.Command;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
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

        public bool AllowOtherLanguage
        {
            get => Model.AllowOtherLanguage;
            set => Model.AllowOtherLanguage = value;
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
        public ICommand BuildCommand { get; private set; }
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
            BuildCommand = new RelayCommand(OnBuild);
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
                    conf["ip"] = ExternalIP;
                    conf["port"] = setting.Port;
                    conf["transfer delay"] = 0;
                    conf["allow other language"] = AllowOtherLanguage;
                    conf["forbidden"] = new JArray();
                    conf["agreement"] = Agreement;
                    conf["admin_mode"] = AdminMode;
                    conf["thread"] = JObject.FromObject(new
                    {
                        logic = 12,
                        io = 12,
                        background = 8
                    });
                    conf["internal"] = JObject.FromObject(new
                    {
                        ip = "127.0.0.1",
                        port = InternalPort
                    });
                    conf["log"] = new JArray("debug", "info", "warn", "fatal");
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
                    File.WriteAllText(Path.Combine([confDir, $"config.login-{i}.json"]), conf.ToString(Formatting.Indented));
                }

                for (int i = 0; i < Game.Count; i++)
                {
                    var setting = Game[i];
                    if (setting.Port == 0)
                        throw new InvalidOperationException($"{i + 1}번째 게임 서버의 포트가 설정되지 않았습니다.");

                    var conf = new JObject();
                    conf["id"] = setting.ID;
                    conf["ip"] = ExternalIP;
                    conf["port"] = setting.Port;
                    conf["name"] = $"game-{setting.ID}";
                    conf["delay"] = 5;
                    conf["thread"] = JObject.FromObject(new
                    {
                        logic = 12,
                        io = 12,
                        background = 8
                    });
                    conf["save"] = SaveInterval;
                    conf["internal"] = JObject.FromObject(new
                    {
                        ip = "127.0.0.1",
                        port = InternalPort
                    });
                    conf["login"] = JObject.FromObject(new
                    {
                        ip = ExternalIP,
                        port = Login[0].Port
                    });
                    conf["amqp"] = JObject.FromObject(new
                    {
                        ip = RabbitMq.IP,
                        port = RabbitMq.Port,
                        uid = RabbitMq.ID,
                        pwd = RabbitMq.PW
                    });
                    conf["log"] = new JArray("debug", "info", "warn", "fatal");
                    File.WriteAllText(Path.Combine([confDir, $"config.game-{setting.ID}.json"]), conf.ToString(Formatting.Indented));
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
                    io = 12,
                    background = 8
                });
                gatewayConf["log"] = new JArray("debug", "info", "warn", "fatal");
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

                File.WriteAllText(Path.Combine([confDir, $"config.gateway.json"]), gatewayConf.ToString(Formatting.Indented));

                var internalConf = new JObject();
                internalConf["Logging"] = new JObject();
                internalConf["Logging"]["LogLevel"] = new JObject();
                internalConf["Logging"]["LogLevel"]["Default"] = "Information";
                internalConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";

                internalConf["ConnectionStrings"] = new JObject();
                internalConf["ConnectionStrings"]["MySql"] = new JObject();
                for (int i = 0; i < MySQL.Count; i++)
                {
                    var db = MySQL[i];
                    internalConf["ConnectionStrings"]["MySql"][(i - 1).ToString()] = $"Server={db.IP};Port={db.Port};User ID={db.ID}; Password={db.PW}; Database=fb";
                }

                internalConf["Redis"] = new JObject();
                for (int i = 0; i < Redis.Count; i++)
                {
                    var db = Redis[i];
                    var node = new JObject();
                    node["Host"] = db.IP;
                    node["Port"] = db.Port;
                    internalConf["Redis"][(i - 1).ToString()] = node;
                }

                internalConf["RabbitMQ"] = new JObject();
                internalConf["RabbitMQ"]["Host"] = RabbitMq.IP;
                internalConf["RabbitMQ"]["Port"] = RabbitMq.Port;
                internalConf["RabbitMQ"]["Uid"] = RabbitMq.ID;
                internalConf["RabbitMQ"]["Pwd"] = RabbitMq.Port;
                File.WriteAllText(Path.Combine([WorkingDirectory, "build", "dist", "internal", "appsettings.internal.json"]), internalConf.ToString(Formatting.Indented));

                var wbConf = new JObject();
                wbConf["Logging"] = new JObject();
                wbConf["Logging"]["LogLevel"] = new JObject();
                wbConf["Logging"]["LogLevel"]["Default"] = "Information";
                wbConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";

                wbConf["ConnectionStrings"] = new JObject();
                wbConf["ConnectionStrings"]["MySql"] = new JObject();
                for (int i = 0; i < MySQL.Count; i++)
                {
                    var db = MySQL[i];
                    wbConf["ConnectionStrings"]["MySql"][(i - 1).ToString()] = $"Server={db.IP};Port={db.Port};User ID={db.ID}; Password= {db.PW} ; Database=fbfb";
                }

                wbConf["Redis"] = new JObject();
                for (int i = 0; i < Redis.Count; i++)
                {
                    var db = Redis[i];
                    var node = new JObject();
                    node["Host"] = db.IP;
                    node["Port"] = db.Port;
                    wbConf["Redis"][(i - 1).ToString()] = node;
                }
                File.WriteAllText(Path.Combine([WorkingDirectory, "build", "dist", "write-back", "appsettings.write-back.json"]), wbConf.ToString(Formatting.Indented));

                var gateway = new ProcessGroup { Type = ServerType.Gateway };
                gateway.Processes.Add(ExecCPP("gateway.exe", "gateway"));
                Servers.Add(gateway);

                var login = new ProcessGroup { Type = ServerType.Login };
                for (int i = 0; i < Login.Count; i++)
                {
                    login.Processes.Add(ExecCPP("login.exe", $"login-{i}"));
                }
                Servers.Add(login);

                var game = new ProcessGroup { Type = ServerType.Game };
                for (int i = 0; i < Game.Count; i++)
                {
                    game.Processes.Add(ExecCPP("game.exe", $"game-{i}"));
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

        private ServerProcess ExecCPP(string file, string env)
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
                    WorkingDirectory = Path.Combine([WorkingDirectory, "build", "dist"]),
                    FileName = Path.Combine(WorkingDirectory, "build", "dist", file),
                    EnvironmentVariables =
                    {
                        ["KINGDOM_OF_WIND_ENVIRONMENT"] = env,
                    }
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
            };
            process.ErrorDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                sp.Output += (e.Data + Environment.NewLine);
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
            };
            process.ErrorDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                sp.Output += (e.Data + Environment.NewLine);
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

        private void OnBuild(object obj)
        {
            if (BuildProcess != null)
                return;

            if (IsConverting)
                return;

            if (string.IsNullOrEmpty(WorkingDirectory) || Directory.Exists(WorkingDirectory) == false)
                return;

            KillProcesses();

            BuildProcess = new Process
            {
                EnableRaisingEvents = true,
                StartInfo = new ProcessStartInfo
                {
                    WindowStyle = ProcessWindowStyle.Hidden,
                    WorkingDirectory = WorkingDirectory,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    //StandardOutputEncoding = Encoding.UTF8,
                    //StandardErrorEncoding = Encoding.UTF8,
                    FileName = "cmd.exe",
                    Arguments = @"/C mkdir build & pushd build & cmake .. & cmake --build . --config Debug & mkdir dist & XCOPY /s /y gateway\Debug\gateway.exe dist\gateway.* & XCOPY /s /y login\Debug\login.exe dist\login.* & XCOPY /s /y game\Debug\game.exe dist\game.* & popd & dotnet publish internal/internal.csproj -c Release -o build/dist/internal & dotnet publish write-back/write-back.csproj -c Release -o build/dist/write-back & rmdir /s /q build\\dist\\json & ROBOCOPY /NP /NFL game\\json\\ build\\dist\\json\\ & ROBOCOPY /NP /NFL game\\maps\\ build\\dist\\maps\\ & ROBOCOPY /NP /NFL game\\scripts\\ build\\dist\\scripts\\"
                }
            };

            BuildLog = string.Empty;
            BuildProcess.OutputDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                BuildLog += (e.Data + Environment.NewLine);
            };
            BuildProcess.ErrorDataReceived += (sender, e) =>
            {
                if (e.Data == null)
                    return;

                BuildLog += (e.Data + Environment.NewLine);
            };
            BuildProcess.Start();
            BuildProcess.BeginOutputReadLine();
            BuildProcess.BeginErrorReadLine();
            BuildProcess.Exited += (sender, e) =>
            {
                LastBuildDate = DateTime.Now;
                BuildProcess = null;
            };
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
