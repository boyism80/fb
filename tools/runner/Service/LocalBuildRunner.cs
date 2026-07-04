using System.IO;
using System.IO.Compression;

namespace Runner.Service
{
    public sealed class BuildFailedException : Exception
    {
        public string StepName { get; }

        public BuildFailedException(string stepName, string message)
            : base(message)
        {
            StepName = stepName;
        }
    }

    public static class LocalBuildRunner
    {
        private static readonly (string Project, string OutputDir)[] DotnetProjects =
        {
            ("server/internal/internal.csproj", "internal"),
            ("server/write-back/write-back.csproj", "write-back"),
            ("server/log/log.csproj", "log"),
            ("server/marketplace/marketplace.csproj", "marketplace"),
            ("server/admin-tool/admin-tool.csproj", "admin-tool"),
        };

        public static async Task RunAsync(
            string workdir,
            string? vcvarsAllPath,
            Action<string> log,
            CancellationToken cancellationToken = default)
        {
            var buildDir = Path.Combine(workdir, "build");
            var distDir = Path.Combine(buildDir, "dist");
            Directory.CreateDirectory(distDir);

            var cmakeCache = Path.Combine(buildDir, "CMakeCache.txt");
            if (File.Exists(cmakeCache) == false)
            {
                log("[cmake] Configuring CMake..." + Environment.NewLine);
                var configure = await RunToolAsync(
                    workdir,
                    vcvarsAllPath,
                    "cmake",
                    "-S . -B build",
                    TimeSpan.FromMinutes(30),
                    log,
                    cancellationToken);

                if (configure.ExitCode != 0)
                    throw new BuildFailedException("cmake configure", TrimOutput(configure));
            }
            else
            {
                log("[cmake] Using existing CMake cache." + Environment.NewLine);
            }

            log("[cpp] Building C++ (Debug)..." + Environment.NewLine);
            var cppBuild = await RunToolAsync(
                workdir,
                vcvarsAllPath,
                "cmake",
                "--build build --config Debug",
                TimeSpan.FromHours(3),
                log,
                cancellationToken);

            if (cppBuild.ExitCode != 0)
                throw new BuildFailedException("C++ build", TrimOutput(cppBuild));

            log("[copy] Copying executables to build\\dist..." + Environment.NewLine);
            CopyExecutable(buildDir, distDir, @"server\gateway\Debug\gateway.exe", "gateway.exe", log);
            CopyExecutable(buildDir, distDir, @"server\login\Debug\login.exe", "login.exe", log);
            CopyExecutable(buildDir, distDir, @"server\game\Debug\game.exe", "game.exe", log);

            foreach (var (project, outputDir) in DotnetProjects)
            {
                var fullProject = Path.Combine(workdir, project);
                if (File.Exists(fullProject) == false)
                {
                    log($"[dotnet] WARNING: Project not found: {project}{Environment.NewLine}");
                    continue;
                }

                var outPath = Path.Combine(distDir, outputDir);
                if (Directory.Exists(outPath))
                    Directory.Delete(outPath, true);

                log($"[dotnet] Publishing {project}..." + Environment.NewLine);
                var publish = await ProcessRunner.RunAsync(
                    "dotnet",
                    $"publish \"{fullProject}\" -c Release -o \"{outPath}\"",
                    workdir,
                    TimeSpan.FromMinutes(30),
                    line => log(line + Environment.NewLine),
                    cancellationToken);

                if (publish.ExitCode != 0)
                    throw new BuildFailedException($"dotnet publish ({outputDir})", TrimOutput(publish));
            }

            log("[maps] Extracting maps..." + Environment.NewLine);
            var mapsZip = Path.Combine(workdir, "resources", "maps", "maps.zip");
            var mapsOut = Path.Combine(distDir, "maps");
            if (File.Exists(mapsZip))
            {
                if (Directory.Exists(mapsOut))
                    Directory.Delete(mapsOut, true);
                Directory.CreateDirectory(mapsOut);
                ZipFile.ExtractToDirectory(mapsZip, mapsOut, true);
                log($"[maps] Extracted to {mapsOut}{Environment.NewLine}");
            }
            else
            {
                log($"[maps] WARNING: {mapsZip} not found{Environment.NewLine}");
            }

            log("[scripts] Copying scripts..." + Environment.NewLine);
            CopyDirectory(
                Path.Combine(workdir, "server", "game", "scripts"),
                Path.Combine(distDir, "scripts"),
                log,
                "[scripts]");

            log("[json] Copying json..." + Environment.NewLine);
            CopyDirectory(
                Path.Combine(workdir, "server", "game", "json"),
                Path.Combine(distDir, "json"),
                log,
                "[json]");

            log("[build] Local build completed successfully." + Environment.NewLine);
        }

        private static async Task<ProcessResult> RunToolAsync(
            string workdir,
            string? vcvarsAllPath,
            string tool,
            string arguments,
            TimeSpan timeout,
            Action<string> log,
            CancellationToken cancellationToken)
        {
            if (string.IsNullOrWhiteSpace(vcvarsAllPath))
            {
                return await ProcessRunner.RunAsync(
                    tool,
                    arguments,
                    workdir,
                    timeout,
                    line => log(line + Environment.NewLine),
                    cancellationToken);
            }

            var command = $"call \"{vcvarsAllPath}\" x64 && {tool} {arguments}";
            return await ProcessRunner.RunCmdAsync(
                command,
                workdir,
                timeout,
                line => log(line + Environment.NewLine),
                cancellationToken);
        }

        private static void CopyExecutable(string buildDir, string distDir, string relativeSource, string fileName, Action<string> log)
        {
            var source = Path.Combine(buildDir, relativeSource);
            var dest = Path.Combine(distDir, fileName);
            if (File.Exists(source) == false)
                throw new BuildFailedException("copy", $"Missing build output: {source}");

            File.Copy(source, dest, true);
            log($"[copy] {source} -> {dest}{Environment.NewLine}");
        }

        private static void CopyDirectory(string sourceDir, string destDir, Action<string> log, string prefix)
        {
            if (Directory.Exists(sourceDir) == false)
            {
                log($"{prefix} WARNING: Source not found: {sourceDir}{Environment.NewLine}");
                return;
            }

            if (Directory.Exists(destDir))
                Directory.Delete(destDir, true);

            CopyDirectoryRecursive(sourceDir, destDir);
            log($"{prefix} Copied {sourceDir} -> {destDir}{Environment.NewLine}");
        }

        private static void CopyDirectoryRecursive(string sourceDir, string destDir)
        {
            Directory.CreateDirectory(destDir);
            foreach (var file in Directory.GetFiles(sourceDir))
                File.Copy(file, Path.Combine(destDir, Path.GetFileName(file)), true);

            foreach (var dir in Directory.GetDirectories(sourceDir))
                CopyDirectoryRecursive(dir, Path.Combine(destDir, Path.GetFileName(dir)));
        }

        private static string TrimOutput(ProcessResult result)
        {
            var text = (result.StdErr + Environment.NewLine + result.StdOut).Trim();
            if (text.Length > 2000)
                text = text[..2000] + "...";
            return string.IsNullOrWhiteSpace(text) ? $"Exit code {result.ExitCode}" : text;
        }
    }
}
