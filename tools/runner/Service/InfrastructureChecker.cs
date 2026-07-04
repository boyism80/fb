using Runner.ViewModel;
using System.Globalization;
using System.IO;
using System.Text.RegularExpressions;

namespace Runner.Service
{
    public static class InfrastructureChecker
    {
        public static string? VcvarsAllPath { get; private set; }

        private static readonly (string Label, string RelativePath)[] DependencySentinels =
        {
            ("jsoncpp (Debug)", @"dependency\lib\jsoncppd.lib"),
            ("jsoncpp (Release)", @"dependency\lib\jsoncpp.lib"),
            ("rabbitmq-c (Debug)", @"dependency\lib\librabbitmq.4d.lib"),
            ("Lua (Debug)", @"dependency\lib\luad.lib"),
            ("zlib (Debug)", @"dependency\lib\zlibd.lib"),
            ("flatbuffers (Debug)", @"dependency\lib\flatbuffersd.lib"),
            ("jsoncpp headers", @"dependency\include\json"),
            ("rabbitmq headers", @"dependency\include\rabbitmq-c"),
            ("Lua headers", @"dependency\include\lua"),
            ("zlib headers", @"dependency\include\zlib"),
            ("cpp-async headers", @"dependency\include\async"),
            ("aho_corasick headers", @"dependency\include\aho_corasick"),
            ("Boost headers", @"dependency\include\boost"),
        };

        public static async Task<InfraCheckResult> CheckAllAsync(
            Model.MainWindow model,
            IReadOnlyList<InfraCheckItem> items,
            Action<InfraCheckItem>? onItemUpdated = null,
            CancellationToken cancellationToken = default)
        {
            var workdir = model.WorkingDirectory ?? string.Empty;
            var byId = items.ToDictionary(i => i.Id);

            void Touch(InfraCheckItem item)
            {
                onItemUpdated?.Invoke(item);
            }

            async Task<InfraCheckItem> RunAsync(string id, Func<Task> check)
            {
                var item = byId[id];
                item.Status = InfraCheckStatus.Checking;
                Touch(item);
                try
                {
                    await check();
                }
                catch (Exception ex)
                {
                    item.Fail(ex.Message);
                    Touch(item);
                }

                return item;
            }

            if (byId.TryGetValue("workdir", out var workdirItem))
            {
                await RunAsync("workdir", async () =>
                {
                    await Task.CompletedTask;
                    if (string.IsNullOrWhiteSpace(workdir) || Directory.Exists(workdir) == false)
                    {
                        workdirItem.Fail("Working directory does not exist.");
                        Touch(workdirItem);
                        return;
                    }

                    var cmakeLists = Path.Combine(workdir, "CMakeLists.txt");
                    var gitDir = Path.Combine(workdir, ".git");
                    if (File.Exists(cmakeLists) == false)
                    {
                        workdirItem.Fail("CMakeLists.txt not found.", workdir);
                        Touch(workdirItem);
                        return;
                    }

                    if (Directory.Exists(gitDir) == false && File.Exists(gitDir) == false)
                    {
                        workdirItem.Fail(".git not found.", workdir);
                        Touch(workdirItem);
                        return;
                    }

                    workdirItem.Pass(workdir);
                    Touch(workdirItem);
                });

                if (workdirItem.Status == InfraCheckStatus.Fail)
                    return BuildResult(items);
            }

            if (byId.ContainsKey("git"))
            {
                await RunAsync("git", async () =>
                {
                    var item = byId["git"];
                    var result = await ProcessRunner.RunAsync(
                        "git",
                        "--version",
                        workdir,
                        TimeSpan.FromSeconds(10),
                        cancellationToken: cancellationToken);

                    if (result.ExitCode != 0 || string.IsNullOrWhiteSpace(result.StdOut))
                    {
                        item.Fail("git is not available on PATH.");
                        Touch(item);
                        return;
                    }

                    var version = result.StdOut.Split('\n', StringSplitOptions.RemoveEmptyEntries).FirstOrDefault()?.Trim() ?? "git";
                    item.Pass(version);
                    Touch(item);
                });

                if (byId["git"].Status == InfraCheckStatus.Fail)
                    return BuildResult(items);
            }

            if (byId.ContainsKey("submodules"))
            {
                await RunAsync("submodules", async () =>
                {
                    var item = byId["submodules"];
                    var result = await ProcessRunner.RunAsync(
                        "git",
                        "submodule status --recursive",
                        workdir,
                        TimeSpan.FromSeconds(60),
                        cancellationToken: cancellationToken);

                    if (result.ExitCode != 0)
                    {
                        item.Fail("git submodule status failed.", result.StdErr.Trim());
                        Touch(item);
                        return;
                    }

                    var bad = result.StdOut
                        .Split('\n', StringSplitOptions.RemoveEmptyEntries)
                        .Where(line => line.Length > 0 && (line[0] == '-' || line[0] == '+' || line[0] == 'U'))
                        .ToList();

                    if (bad.Count > 0)
                    {
                        item.Fail("Submodules are not initialized or out of sync.", bad.First().Trim());
                        Touch(item);
                        return;
                    }

                    item.Pass("up to date");
                    Touch(item);
                });
            }

            if (byId.ContainsKey("update_modules_script"))
            {
                await RunAsync("update_modules_script", async () =>
                {
                    var item = byId["update_modules_script"];
                    await Task.CompletedTask;
                    var script = Path.Combine(workdir, "tools", "update-modules.bat");
                    if (File.Exists(script) == false)
                    {
                        item.Fail("update-modules.bat not found.", script);
                        Touch(item);
                        return;
                    }

                    item.Pass(script);
                    Touch(item);
                });
            }

            if (byId.ContainsKey("cmake"))
            {
                await RunAsync("cmake", async () =>
                {
                    var item = byId["cmake"];
                    var result = await ProcessRunner.RunAsync(
                        "cmake",
                        "--version",
                        workdir,
                        TimeSpan.FromSeconds(15),
                        cancellationToken: cancellationToken);

                    if (result.ExitCode != 0)
                    {
                        item.Fail("cmake is not available on PATH.");
                        Touch(item);
                        return;
                    }

                    var match = Regex.Match(result.StdOut, @"cmake version (\d+)\.(\d+)");
                    if (!match.Success)
                    {
                        item.Fail("Could not parse cmake version.");
                        Touch(item);
                        return;
                    }

                    var major = int.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture);
                    var minor = int.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture);
                    if (major < 3 || (major == 3 && minor < 28))
                    {
                        item.Fail($"CMake 3.28 or later required (found {major}.{minor}).");
                        Touch(item);
                        return;
                    }

                    item.Pass($"{major}.{minor}");
                    Touch(item);
                });
            }

            if (byId.ContainsKey("msvc"))
            {
                await RunAsync("msvc", async () =>
                {
                    var item = byId["msvc"];
                    await Task.CompletedTask;
                    VcvarsAllPath = null;

                    var programFiles = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86);
                    var vswhere = Path.Combine(programFiles, "Microsoft Visual Studio", "Installer", "vswhere.exe");
                    if (File.Exists(vswhere) == false)
                    {
                        item.Fail("vswhere.exe not found. Install Visual Studio 2022 or later with C++ workload.");
                        Touch(item);
                        return;
                    }

                    var result = await ProcessRunner.RunAsync(
                        vswhere,
                        "-latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath",
                        workdir,
                        TimeSpan.FromSeconds(15),
                        cancellationToken: cancellationToken);

                    var installPath = result.StdOut.Trim();
                    if (result.ExitCode != 0 || string.IsNullOrWhiteSpace(installPath))
                    {
                        item.Fail("Visual Studio 2022 or later with C++ tools not found.");
                        Touch(item);
                        return;
                    }

                    var vcvars = Path.Combine(installPath, "VC", "Auxiliary", "Build", "vcvarsall.bat");
                    if (File.Exists(vcvars) == false)
                    {
                        item.Fail("vcvarsall.bat not found.", installPath);
                        Touch(item);
                        return;
                    }

                    VcvarsAllPath = vcvars;
                    item.Pass("VS 2022 or later (C++)");
                    Touch(item);
                });
            }

            if (byId.ContainsKey("dotnet"))
            {
                await RunAsync("dotnet", async () =>
                {
                    var item = byId["dotnet"];
                    var result = await ProcessRunner.RunAsync(
                        "dotnet",
                        "--list-sdks",
                        workdir,
                        TimeSpan.FromSeconds(15),
                        cancellationToken: cancellationToken);

                    if (result.ExitCode != 0)
                    {
                        item.Fail(".NET SDK not found on PATH.");
                        Touch(item);
                        return;
                    }

                    string? bestSdk = null;
                    Version? bestVersion = null;

                    foreach (var line in result.StdOut.Split('\n', StringSplitOptions.RemoveEmptyEntries))
                    {
                        var sdk = line.Trim().Split(' ')[0];
                        var core = sdk.Split('-')[0];
                        if (Version.TryParse(core, out var version) == false || version.Major < 8)
                            continue;

                        if (bestVersion == null || version > bestVersion)
                        {
                            bestVersion = version;
                            bestSdk = sdk;
                        }
                    }

                    if (bestSdk == null)
                    {
                        item.Fail(".NET SDK 8.0 or later is required.");
                        Touch(item);
                        return;
                    }

                    item.Pass(bestSdk);
                    Touch(item);
                });
            }

            if (byId.ContainsKey("disk"))
            {
                await RunAsync("disk", async () =>
                {
                    var item = byId["disk"];
                    await Task.CompletedTask;
                    if (string.IsNullOrWhiteSpace(workdir))
                    {
                        item.Fail("Working directory is not set.");
                        Touch(item);
                        return;
                    }

                    var buildPath = Path.Combine(workdir, "build");
                    var root = Path.GetPathRoot(Path.GetFullPath(buildPath));
                    if (string.IsNullOrWhiteSpace(root))
                    {
                        item.Fail("Could not resolve drive for build directory.");
                        Touch(item);
                        return;
                    }

                    var drive = new DriveInfo(root);
                    const long required = 10L * 1024 * 1024 * 1024;
                    if (drive.AvailableFreeSpace < required)
                    {
                        var freeGb = drive.AvailableFreeSpace / (1024.0 * 1024 * 1024);
                        item.Fail($"At least 10 GB free required ({freeGb:F1} GB available on {drive.Name}).");
                        Touch(item);
                        return;
                    }

                    var freeGbPass = drive.AvailableFreeSpace / (1024.0 * 1024 * 1024);
                    item.Pass($"{freeGbPass:F1} GB free on {drive.Name}");
                    Touch(item);
                });
            }

            if (byId.ContainsKey("dependency"))
            {
                await RunAsync("dependency", async () =>
                {
                    var item = byId["dependency"];
                    await Task.CompletedTask;
                    item.Detail = @"dependency\";

                    var missing = new List<string>();
                    foreach (var (label, relative) in DependencySentinels)
                    {
                        var full = Path.Combine(workdir, relative);
                        if (File.Exists(full) == false && Directory.Exists(full) == false)
                            missing.Add(label);
                    }

                    var boostLibDir = Path.Combine(workdir, @"dependency\lib\boost");
                    if (Directory.Exists(boostLibDir) == false ||
                        Directory.EnumerateFiles(boostLibDir, "*.lib").Any() == false)
                    {
                        missing.Add("Boost libs");
                    }

                    if (missing.Count > 0)
                    {
                        item.Detail = "미준비 — update-modules.bat 필요";
                        item.Fail(
                            "Run tools\\update-modules.bat in Developer PowerShell for Visual Studio 2022 or later.",
                            string.Join(", ", missing.Take(3)));
                        Touch(item);
                        return;
                    }

                    item.Pass(@"dependency\");
                    Touch(item);
                });
            }

            foreach (var item in items.Where(i => i.Id.StartsWith("mysql_", StringComparison.Ordinal)))
            {
                await RunAsync(item.Id, async () =>
                {
                    await Task.CompletedTask;
                    var index = int.Parse(item.Id["mysql_".Length..], CultureInfo.InvariantCulture);
                    if (index < 0 || index >= model.MySQL.Count)
                    {
                        item.Fail("MySQL entry not found.");
                        Touch(item);
                        return;
                    }

                    var mysql = model.MySQL[index];
                    item.Detail = $"{mysql.IP}:{mysql.Port}";
                    if (NetworkHelper.TcpPing(mysql.IP, mysql.Port) == false)
                    {
                        item.Fail($"Cannot connect to {mysql.IP}:{mysql.Port}");
                        Touch(item);
                        return;
                    }

                    item.Pass(item.Detail);
                    Touch(item);
                });
            }

            foreach (var item in items.Where(i => i.Id.StartsWith("redis_", StringComparison.Ordinal)))
            {
                await RunAsync(item.Id, async () =>
                {
                    await Task.CompletedTask;
                    var index = int.Parse(item.Id["redis_".Length..], CultureInfo.InvariantCulture);
                    if (index < 0 || index >= model.Redis.Count)
                    {
                        item.Fail("Redis entry not found.");
                        Touch(item);
                        return;
                    }

                    var redis = model.Redis[index];
                    item.Detail = $"{redis.IP}:{redis.Port}";
                    if (NetworkHelper.TcpPing(redis.IP, redis.Port) == false)
                    {
                        item.Fail($"Cannot connect to {redis.IP}:{redis.Port}");
                        Touch(item);
                        return;
                    }

                    item.Pass(item.Detail);
                    Touch(item);
                });
            }

            if (byId.ContainsKey("rabbitmq"))
            {
                await RunAsync("rabbitmq", async () =>
                {
                    var item = byId["rabbitmq"];
                    await Task.CompletedTask;
                    var mq = model.RabbitMq;
                    item.Detail = $"{mq.IP}:{mq.Port}";
                    if (NetworkHelper.TcpPing(mq.IP, mq.Port) == false)
                    {
                        item.Fail($"Cannot connect to {mq.IP}:{mq.Port}");
                        Touch(item);
                        return;
                    }

                    item.Pass(item.Detail);
                    Touch(item);
                });
            }

            return BuildResult(items);
        }

        private static InfraCheckResult BuildResult(IReadOnlyList<InfraCheckItem> items)
        {
            var required = items.Where(i => i.Required).ToList();
            var passed = required.Count(i => i.Status == InfraCheckStatus.Pass);
            return new InfraCheckResult
            {
                Items = items,
                RequiredTotal = required.Count,
                RequiredPassed = passed
            };
        }
    }
}
