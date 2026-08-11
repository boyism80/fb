using Microsoft.Win32;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Runner.Command;
using Runner.Service;
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
using System.Windows.Data;
using System.Windows.Input;

namespace Runner.ViewModel
{
    public enum ServerType
    {
        Gateway,
        Game,
        Login,
        Internal,
        WriteBack,
        Log,
        Marketplace,
        Matchmaking,
        AdminTool
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
        public string ClientVersions
        {
            get => string.Join(",", Model.ClientVersions ?? new List<ushort>());
            set
            {
                var parsed = new List<ushort>();
                if (string.IsNullOrWhiteSpace(value) == false)
                {
                    foreach (var part in value.Split(new[] { ',', ' ', ';' }, StringSplitOptions.RemoveEmptyEntries))
                    {
                        if (ushort.TryParse(part.Trim(), out var v))
                            parsed.Add(v);
                    }
                }
                Model.ClientVersions = parsed.Count > 0 ? parsed : new List<ushort> { 550, 565 };
            }
        }
        public byte ClientNation
        {
            get => Model.ClientNation;
            set => Model.ClientNation = value;
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
        public ushort MarketplacePort
        {
            get => Model.Marketplace.Port;
            set => Model.Marketplace.Port = value;
        }
        public ushort MatchmakingPort
        {
            get => Model.Matchmaking.Port;
            set => Model.Matchmaking.Port = value;
        }
        public ushort AdminToolPort
        {
            get => Model.AdminTool.Port;
            set => Model.AdminTool.Port = value;
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
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsEnableRun)));
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RunButtonText)));
                RecomputeCanBuild();
            }
        }

        private bool _isCheckingInfrastructure;
        public bool IsCheckingInfrastructure
        {
            get => _isCheckingInfrastructure;
            set
            {
                _isCheckingInfrastructure = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsCheckingInfrastructure)));
                RecomputeCanBuild();
            }
        }

        public ObservableCollection<InfraCheckItem> InfrastructureChecks { get; } = new();

        private string _infrastructureSummary = string.Empty;
        public string InfrastructureSummary
        {
            get => _infrastructureSummary;
            set
            {
                _infrastructureSummary = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(InfrastructureSummary)));
            }
        }

        private bool _canBuild;
        public bool CanBuild
        {
            get => _canBuild;
            set
            {
                _canBuild = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(CanBuild)));
            }
        }

        private bool _infrastructureLoaded;
        private CancellationTokenSource? _infraCheckCts;
        private InfraCheckResult? _lastInfraResult;
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

        public ObservableCollection<SettingNavItem> SettingNavItems { get; } = new();
        public ICollectionView SettingNavView { get; private set; }
        public SettingSection SelectedSettingSection { get; set; } = SettingSection.Directory;

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
            InitSettingNav();
            SetMinimizeCommand = new RelayCommand(OnSetMinimize);
            SetMaximizeCommand = new RelayCommand(OnSetMaximize);
            CloseCommand = new RelayCommand(OnClose);
            NewMySQL = new RelayCommand(OnNewMySQL);
            NewRedis = new RelayCommand(OnNewRedis);
            NewLogin = new RelayCommand(OnNewLogin);
            NewGame = new RelayCommand(OnNewGame);
            FindWorkingDirectory = new RelayCommand(OnFindWorkingDirectory);
            PatchCommand = new RelayCommand(OnPatch);
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

        private void InitSettingNav()
        {
            SettingNavItems.Add(new SettingNavItem("일반", "Directory", SettingSection.Directory));
            SettingNavItems.Add(new SettingNavItem("인프라", "Infrastructure", SettingSection.Infrastructure));
            SettingNavItems.Add(new SettingNavItem("서버", "Gateway", SettingSection.Gateway));
            SettingNavItems.Add(new SettingNavItem("서버", "Login", SettingSection.Login));
            SettingNavItems.Add(new SettingNavItem("서버", "Game", SettingSection.Game));
            SettingNavItems.Add(new SettingNavItem("서버", "Internal", SettingSection.Internal));
            SettingNavItems.Add(new SettingNavItem("서버", "Marketplace", SettingSection.Marketplace));
            SettingNavItems.Add(new SettingNavItem("서버", "Matchmaking", SettingSection.Matchmaking));
            SettingNavItems.Add(new SettingNavItem("서버", "Admin Tool", SettingSection.AdminTool));

            SettingNavView = CollectionViewSource.GetDefaultView(SettingNavItems);
            SettingNavView.GroupDescriptions.Add(new PropertyGroupDescription(nameof(SettingNavItem.Group)));
        }

        private void Servers_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsRunning)));
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsEnableSettingTab)));
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsEnableEdit)));
            RecomputeCanBuild();
        }

        public async Task EnsureInfrastructureCheckedAsync()
        {
            if (_infrastructureLoaded)
                return;

            _infrastructureLoaded = true;
            await RefreshInfrastructureAsync();
        }

        public async Task RefreshInfrastructureAsync()
        {
            if (IsCheckingInfrastructure)
                return;

            _infraCheckCts?.Cancel();
            _infraCheckCts = new CancellationTokenSource();
            var ct = _infraCheckCts.Token;

            IsCheckingInfrastructure = true;
            InfrastructureSummary = "인프라 확인 중...";

            var items = BuildInfrastructureCheckItems();
            Application.Current.Dispatcher.Invoke(() =>
            {
                InfrastructureChecks.Clear();
                foreach (var item in items)
                {
                    item.Status = InfraCheckStatus.Checking;
                    InfrastructureChecks.Add(item);
                }
            });

            InfraCheckResult? result = null;

            try
            {
                result = await Task.Run(async () =>
                    await InfrastructureChecker.CheckAllAsync(Model, items, null, ct), ct);

                Application.Current.Dispatcher.Invoke(() =>
                {
                    InfrastructureChecks.Clear();
                    foreach (var item in items)
                        InfrastructureChecks.Add(item);
                });
            }
            catch (OperationCanceledException)
            {
                InfrastructureSummary = "인프라 확인이 취소되었습니다.";
            }
            catch (Exception ex)
            {
                InfrastructureSummary = $"인프라 확인 오류: {ex.Message}";
            }
            finally
            {
                IsCheckingInfrastructure = false;
            }

            if (result != null)
                Application.Current.Dispatcher.Invoke(() => UpdateInfrastructureSummary(result));
        }

        private List<InfraCheckItem> BuildInfrastructureCheckItems()
        {
            var items = new List<InfraCheckItem>
            {
                new("workdir", "Working Directory", InfraCheckGroup.Repository),
                new("git", "Git", InfraCheckGroup.Repository),
                new("submodules", "Git submodules", InfraCheckGroup.Repository),
                new("update_modules_script", "update-modules.bat", InfraCheckGroup.Repository),
                new("cmake", "CMake 3.28 or later", InfraCheckGroup.Toolchain),
                new("msvc", "MSVC (VS 2022 or later)", InfraCheckGroup.Toolchain),
                new("dotnet", ".NET SDK 8.0 or later", InfraCheckGroup.Toolchain),
                new("disk", "Disk space", InfraCheckGroup.Toolchain),
                new("dependency", "Native dependencies", InfraCheckGroup.NativeArtifacts),
            };

            foreach (var mysql in MySQL)
            {
                var index = MySQL.IndexOf(mysql);
                items.Add(new InfraCheckItem($"mysql_{index}", $"MySQL ({mysql.Name})", InfraCheckGroup.Runtime)
                {
                    Detail = $"{mysql.IP}:{mysql.Port}"
                });
            }

            for (var i = 0; i < Redis.Count; i++)
            {
                var redis = Redis[i];
                items.Add(new InfraCheckItem($"redis_{i}", $"Redis ({redis.IP})", InfraCheckGroup.Runtime)
                {
                    Detail = $"{redis.IP}:{redis.Port}"
                });
            }

            items.Add(new InfraCheckItem("rabbitmq", "RabbitMQ", InfraCheckGroup.Runtime)
            {
                Detail = $"{RabbitMq.IP}:{RabbitMq.Port}"
            });

            return items;
        }

        private void UpdateInfrastructureSummary(InfraCheckResult result)
        {
            _lastInfraResult = result;

            var warnings = result.Items.Count(i => i.Status == InfraCheckStatus.Warning);
            if (result.CanBuild)
            {
                InfrastructureSummary = warnings > 0
                    ? $"{result.RequiredPassed}/{result.RequiredTotal} 통과 ({warnings} 경고)"
                    : $"{result.RequiredPassed}/{result.RequiredTotal} 통과";
            }
            else
            {
                InfrastructureSummary = $"{result.RequiredPassed}/{result.RequiredTotal} 통과 - 빌드 불가";
            }

            RecomputeCanBuild();
        }

        private void RecomputeCanBuild()
        {
            CanBuild = (_lastInfraResult?.CanBuild ?? false)
                && !IsConverting
                && !IsRunning
                && !IsCheckingInfrastructure;
        }

        private string GetBuildBlockReason()
        {
            var failures = InfrastructureChecks
                .Where(i => i.Required && i.Status == InfraCheckStatus.Fail)
                .Select(i => string.IsNullOrWhiteSpace(i.Message) ? i.Name : $"{i.Name}: {i.Message}")
                .ToList();

            if (failures.Count > 0)
                return string.Join(Environment.NewLine, failures);

            if (IsRunning)
                return "서버가 실행 중입니다. 서버를 종료한 후 다시 시도하세요.";

            if (IsConverting)
                return "패치 또는 빌드가 진행 중입니다.";

            if (IsCheckingInfrastructure)
                return "인프라 확인이 진행 중입니다.";

            if (_lastInfraResult == null)
                return "인프라 확인이 아직 완료되지 않았습니다.";

            if (_lastInfraResult.CanBuild == false)
                return $"필수 인프라 {_lastInfraResult.RequiredPassed}/{_lastInfraResult.RequiredTotal} 항목만 통과했습니다.";

            return "빌드를 시작할 수 없습니다.";
        }

        private void OnBuild(object obj)
        {
            _ = OnBuildAsync();
        }

        private async Task OnBuildAsync()
        {
            if (IsConverting || IsCheckingInfrastructure)
                return;

            if (IsRunning)
            {
                MessageBox.Show("서버가 실행 중입니다. 빌드하기 전에 서버를 종료하세요.", "빌드");
                return;
            }

            await RefreshInfrastructureAsync();

            if (CanBuild == false)
            {
                MessageBox.Show(
                    GetBuildBlockReason(),
                    "빌드 불가",
                    MessageBoxButton.OK,
                    MessageBoxImage.Warning);
                return;
            }

            var confirm = MessageBox.Show(
                "build\\dist 폴더의 기존 바이너리를 로컬에서 빌드한 결과로 덮어씁니다." + Environment.NewLine + Environment.NewLine +
                "• 예상 소요 시간: 30분~1시간 이상 (최초 native 의존성 빌드 시 더 길 수 있음)" + Environment.NewLine +
                "• gateway.exe, login.exe, game.exe 및 .NET 서비스가 교체됩니다" + Environment.NewLine +
                "• 진행 중에는 패치/시작 버튼을 사용할 수 없습니다" + Environment.NewLine + Environment.NewLine +
                "계속하시겠습니까?",
                "로컬 빌드 확인",
                MessageBoxButton.YesNo,
                MessageBoxImage.Question,
                MessageBoxResult.No);

            if (confirm != MessageBoxResult.Yes)
                return;

            if (string.IsNullOrWhiteSpace(WorkingDirectory) || Directory.Exists(WorkingDirectory) == false)
            {
                MessageBox.Show("Working directory가 올바르지 않습니다.", "빌드");
                return;
            }

            IsConverting = true;
            SelectedProcess = null;
            BuildLog = string.Empty;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MainText)));

            try
            {
                await Task.Run(async () =>
                {
                    await LocalBuildRunner.RunAsync(
                        WorkingDirectory,
                        InfrastructureChecker.VcvarsAllPath,
                        line => Application.Current.Dispatcher.Invoke(() => BuildLog += line),
                        CancellationToken.None);
                });

                LastBuildDate = DateTime.Now;
                BuildLog += $"Build completed at {LastBuildDate}{Environment.NewLine}";
                MessageBox.Show("로컬 빌드가 완료되었습니다.", "완료");
            }
            catch (BuildFailedException ex)
            {
                BuildLog += $"Build failed ({ex.StepName}): {ex.Message}{Environment.NewLine}";
                MessageBox.Show($"빌드 실패 ({ex.StepName}):{Environment.NewLine}{ex.Message}", "오류", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            catch (Exception ex)
            {
                BuildLog += $"Build error: {ex}{Environment.NewLine}";
                MessageBox.Show(ex.Message, "오류", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            finally
            {
                IsConverting = false;
            }
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
                    Arguments = @"/C pushd tools & call update-data.bat true & popd & robocopy /NP /NFL server\game\json\ build\dist\json\ & robocopy /NP /NFL server\internal\json\ build\dist\internal\json\ & robocopy /NP /NFL server\matchmaking\json\ build\dist\matchmaking\json\"
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

                if (MarketplacePort == 0)
                    throw new InvalidOperationException("Marketplace 서버 포트가 설정되지 않았습니다.");

                if (MatchmakingPort == 0)
                    throw new InvalidOperationException("Matchmaking 서버 포트가 설정되지 않았습니다.");

                if (AdminToolPort == 0)
                    throw new InvalidOperationException("Admin Tool 서버 포트가 설정되지 않았습니다.");

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

                if (string.IsNullOrEmpty(WorkingDirectory) || Directory.Exists(WorkingDirectory) == false)
                    throw new InvalidOperationException("Working directory가 설정되지 않았거나 존재하지 않습니다.");

                EnsureDistJsonDirectory(
                    Path.Combine(WorkingDirectory, "build", "dist", "json"),
                    "게임");
                EnsureDistJsonDirectory(
                    Path.Combine(WorkingDirectory, "build", "dist", "internal", "json"),
                    "Internal");
                EnsureDistJsonDirectory(
                    Path.Combine(WorkingDirectory, "build", "dist", "matchmaking", "json"),
                    "Matchmaking");

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
                    conf["http"] = JObject.FromObject(new
                    {
                        max_concurrent = 128
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
                        port = MarketplacePort
                    });
                    conf["matchmaking"] = JObject.FromObject(new
                    {
                        ip = ExternalIP,
                        port = MatchmakingPort
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
                gatewayConf["http"] = JObject.FromObject(new
                {
                    max_concurrent = 128
                });
                gatewayConf["client"] = JObject.FromObject(new
                {
                    versions = Gateway.Model.ClientVersions ?? new List<ushort> { 550, 565 },
                    nation = Gateway.ClientNation
                });

                File.WriteAllText(Path.Combine([gatewayDir, $"config_gateway.json"]), gatewayConf.ToString(Formatting.Indented));

                var internalConf = new JObject();
                internalConf["Database"] = JObject.FromObject(new { AutoMigration = true });
                internalConf["Logging"] = new JObject();
                internalConf["Logging"]["LogLevel"] = new JObject();
                internalConf["Logging"]["LogLevel"]["Default"] = "Information";
                internalConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";

                var internalMySql = BuildMySqlWithUnified();
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

                var wbMySql = BuildMySqlWorldOnly();
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

                var logConf = new JObject();
                logConf["Database"] = JObject.FromObject(new { AutoMigration = true });
                logConf["World"] = 1;
                logConf["Logging"] = new JObject();
                logConf["Logging"]["LogLevel"] = new JObject();
                logConf["Logging"]["LogLevel"]["Default"] = "Information";
                logConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";

                var logMySql = BuildLogMySql();
                logConf["ConnectionStrings"] = new JObject();
                logConf["ConnectionStrings"]["MySql"] = logMySql;

                var logRabbit = new JObject();
                logRabbit["Internal"] = JObject.FromObject(new { Host = RabbitMq.IP, Port = RabbitMq.Port, Uid = RabbitMq.ID, Pwd = RabbitMq.PW });
                logRabbit["Log"] = JObject.FromObject(new { Host = RabbitMq.IP, Port = RabbitMq.Port, Uid = RabbitMq.ID, Pwd = RabbitMq.PW });
                logConf["RabbitMQ"] = logRabbit;
                File.WriteAllText(Path.Combine([WorkingDirectory, "build", "dist", "log", "appsettings.log.json"]), logConf.ToString(Formatting.Indented));

                var marketplaceConf = new JObject();
                marketplaceConf["Database"] = JObject.FromObject(new { AutoMigration = true });
                marketplaceConf["Logging"] = new JObject();
                marketplaceConf["Logging"]["LogLevel"] = new JObject();
                marketplaceConf["Logging"]["LogLevel"]["Default"] = "Information";
                marketplaceConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";
                marketplaceConf["ConnectionStrings"] = new JObject();
                marketplaceConf["ConnectionStrings"]["MySql"] = internalMySql.DeepClone();
                marketplaceConf["Redis"] = internalRedis.DeepClone();
                marketplaceConf["RabbitMQ"] = internalRabbit.DeepClone();
                marketplaceConf["Log"] = JObject.FromObject(new { Enabled = true, ServerId = "0", ServerName = "marketplace" });
                marketplaceConf["Marketplace"] = JObject.FromObject(new { UseSharding = false });
                marketplaceConf["Urls"] = $"http://127.0.0.1:{MarketplacePort}";
                File.WriteAllText(Path.Combine([WorkingDirectory, "build", "dist", "marketplace", "appsettings.marketplace.json"]), marketplaceConf.ToString(Formatting.Indented));

                var matchmakingConf = new JObject();
                matchmakingConf["Logging"] = new JObject();
                matchmakingConf["Logging"]["LogLevel"] = new JObject();
                matchmakingConf["Logging"]["LogLevel"]["Default"] = "Information";
                matchmakingConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";
                matchmakingConf["RabbitMQ"] = new JObject();
                matchmakingConf["RabbitMQ"]["Internal"] = JObject.FromObject(new { Host = RabbitMq.IP, Port = RabbitMq.Port, Uid = RabbitMq.ID, Pwd = RabbitMq.PW });
                matchmakingConf["Matchmaking"] = JObject.FromObject(new
                {
                    TickIntervalMs = 500,
                    BaseSkillTolerance = 1.0,
                    SkillTolerancePerSecond = 0.1,
                    MaxSkillTolerance = 5.0,
                    EffectiveMuSigmaFactor = 3.0,
                    SkillBucketWidth = 1.0,
                    ConfirmTimeoutSeconds = 30
                });
                matchmakingConf["Urls"] = $"http://127.0.0.1:{MatchmakingPort}";
                File.WriteAllText(Path.Combine([WorkingDirectory, "build", "dist", "matchmaking", "appsettings.matchmaking.json"]), matchmakingConf.ToString(Formatting.Indented));

                var adminToolConf = new JObject();
                adminToolConf["DetailedErrors"] = true;
                adminToolConf["Logging"] = new JObject();
                adminToolConf["Logging"]["LogLevel"] = new JObject();
                adminToolConf["Logging"]["LogLevel"]["Default"] = "Information";
                adminToolConf["Logging"]["LogLevel"]["Microsoft.AspNetCore"] = "Warning";
                adminToolConf["ConnectionStrings"] = new JObject();
                adminToolConf["ConnectionStrings"]["MySql"] = internalMySql.DeepClone();
                adminToolConf["Redis"] = internalRedis.DeepClone();
                adminToolConf["RabbitMQ"] = internalRabbit.DeepClone();
                adminToolConf["WorldServers"] = new JObject
                {
                    ["1"] = Login[0].Name
                };
                adminToolConf["Log"] = JObject.FromObject(new { Enabled = true, ServerId = "0", ServerName = "admin-tool" });
                adminToolConf["Security"] = JObject.FromObject(new { ElevationSecret = "admin" });
                File.WriteAllText(Path.Combine([WorkingDirectory, "build", "dist", "admin-tool", "appsettings.admin-tool.json"]), adminToolConf.ToString(Formatting.Indented));

                var inter = new ProcessGroup { Type = ServerType.Internal };
                inter.Processes.Add(ExecDotNet("internal", InternalPort));
                Servers.Add(inter);

                var logGroup = new ProcessGroup { Type = ServerType.Log };
                logGroup.Processes.Add(ExecDotNet("log"));
                Servers.Add(logGroup);

                var wb = new ProcessGroup { Type = ServerType.WriteBack };
                wb.Processes.Add(ExecDotNet("write-back"));
                Servers.Add(wb);

                var marketplaceGroup = new ProcessGroup { Type = ServerType.Marketplace };
                marketplaceGroup.Processes.Add(ExecDotNet("marketplace", MarketplacePort));
                Servers.Add(marketplaceGroup);

                var matchmakingGroup = new ProcessGroup { Type = ServerType.Matchmaking };
                matchmakingGroup.Processes.Add(ExecDotNet("matchmaking", MatchmakingPort));
                Servers.Add(matchmakingGroup);

                var adminToolGroup = new ProcessGroup { Type = ServerType.AdminTool };
                adminToolGroup.Processes.Add(ExecDotNet("admin-tool", AdminToolPort));
                Servers.Add(adminToolGroup);

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

        private static void EnsureDistJsonDirectory(string jsonDir, string displayName)
        {
            const string updateDataHint =
                "Runner 메뉴에서 파일 → 업데이트 → 데이터를 실행하세요.";

            if (Directory.Exists(jsonDir) == false)
            {
                throw new InvalidOperationException(
                    $"{displayName} JSON 폴더가 없습니다.{Environment.NewLine}{jsonDir}{Environment.NewLine}{Environment.NewLine}{updateDataHint}{Environment.NewLine}또는 패치/빌드 후 다시 시작하세요.");
            }

            if (Directory.EnumerateFiles(jsonDir, "*.json", SearchOption.TopDirectoryOnly).Any() == false)
            {
                throw new InvalidOperationException(
                    $"{displayName} JSON 폴더에 .json 파일이 없습니다.{Environment.NewLine}{jsonDir}{Environment.NewLine}{Environment.NewLine}{updateDataHint}");
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

            await Task.Run(() =>
            {
                using var fs = new FileStream(zipPath, FileMode.Open, FileAccess.Read, FileShare.Read);
                using var archive = new ZipArchive(fs, ZipArchiveMode.Read, leaveOpen: false);

                var entries = archive.Entries
                    .Where(e => !string.IsNullOrEmpty(e.Name))
                    .ToList();

                var totalFiles = entries.Count;
                progressCallback(0, totalFiles, string.Empty);

                var extractedCount = 0;
                foreach (var entry in entries)
                {
                    var destinationPath = Path.Combine(destDir, entry.FullName);
                    var destinationDir = Path.GetDirectoryName(destinationPath);
                    if (string.IsNullOrEmpty(destinationDir) == false)
                        Directory.CreateDirectory(destinationDir);

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

                    extractedCount++;
                    progressCallback(extractedCount, totalFiles, entry.FullName);
                }
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

                    long lastExtractProgressTicks = 0;
                    const int extractProgressIntervalMs = 300;

                    await ExtractZipAsync(zipPath, distDir, (count, total, entryName) =>
                    {
                        if (count == 0)
                        {
                            _ = Application.Current.Dispatcher.InvokeAsync(() =>
                            {
                                BuildLog += $"Extracting {total} files...{Environment.NewLine}";
                            });
                            return;
                        }

                        var now = Environment.TickCount64;
                        var isComplete = count >= total;
                        if (!isComplete && now - lastExtractProgressTicks < extractProgressIntervalMs)
                            return;

                        lastExtractProgressTicks = now;
                        _ = Application.Current.Dispatcher.InvokeAsync(() =>
                        {
                            BuildLog += isComplete
                                ? $"Extracted {count}/{total} (complete){Environment.NewLine}"
                                : $"Extracted {count}/{total}{Environment.NewLine}";
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
                        var sourceDir = Path.Combine(WorkingDirectory, "server", "game", sub);
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

        private const string RunnerWorldId = "1";

        private static string BuildMySqlConnectionString(MySqlConnection db, string database) =>
            $"Server={db.IP};Port={db.Port};User ID={db.ID}; Password={db.PW}; Database={database}";

        private JObject BuildMySqlWithUnified()
        {
            var mySql = new JObject();
            if (MySQL.Count == 0)
                return mySql;

            var globalHost = MySQL[0];
            mySql["unified"] = BuildMySqlConnectionString(globalHost, "fb-unified");
            mySql["worlds"] = new JObject { [RunnerWorldId] = BuildMySqlWorld(globalHost) };
            return mySql;
        }

        private JObject BuildMySqlWorldOnly()
        {
            var mySql = new JObject();
            if (MySQL.Count == 0)
                return mySql;

            mySql["worlds"] = new JObject { [RunnerWorldId] = BuildMySqlWorld(MySQL[0]) };
            return mySql;
        }

        private JObject BuildMySqlWorld(MySqlConnection globalHost)
        {
            var world = new JObject
            {
                ["global"] = BuildMySqlConnectionString(globalHost, "fb-1-global"),
            };

            var data = new JArray();
            for (var i = 1; i < MySQL.Count; i++)
                data.Add(BuildMySqlConnectionString(MySQL[i], $"fb-1-data-{i - 1}"));

            if (data.Count > 0)
                world["data"] = data;

            return world;
        }

        private JObject BuildLogMySql()
        {
            var mySql = new JObject();
            if (MySQL.Count == 0)
                return mySql;

            var globalHost = MySQL[0];
            var world = new JObject
            {
                ["global"] = BuildMySqlConnectionString(globalHost, "fb-1-global"),
                ["data"] = new JArray
                {
                    BuildMySqlConnectionString(globalHost, "fb-1-log-0"),
                },
            };
            mySql["worlds"] = new JObject { [RunnerWorldId] = world };
            return mySql;
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
            _infraCheckCts?.Cancel();
            _infraCheckCts?.Dispose();
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
