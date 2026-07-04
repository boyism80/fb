using System.Diagnostics;
using System.Text;

namespace Runner.Service
{
    public sealed class ProcessResult
    {
        public int ExitCode { get; init; }
        public string StdOut { get; init; } = string.Empty;
        public string StdErr { get; init; } = string.Empty;
        public bool TimedOut { get; init; }
        public bool Started { get; init; } = true;
    }

    public static class ProcessRunner
    {
        public static async Task<ProcessResult> RunAsync(
            string fileName,
            string arguments,
            string workingDirectory,
            TimeSpan timeout,
            Action<string>? onOutput = null,
            CancellationToken cancellationToken = default)
        {
            var stdout = new StringBuilder();
            var stderr = new StringBuilder();

            using var process = new Process
            {
                EnableRaisingEvents = true,
                StartInfo = new ProcessStartInfo
                {
                    FileName = fileName,
                    Arguments = arguments,
                    WorkingDirectory = workingDirectory,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                    StandardOutputEncoding = Encoding.UTF8,
                    StandardErrorEncoding = Encoding.UTF8,
                }
            };

            process.OutputDataReceived += (_, e) =>
            {
                if (e.Data == null)
                    return;
                stdout.AppendLine(e.Data);
                onOutput?.Invoke(e.Data);
            };
            process.ErrorDataReceived += (_, e) =>
            {
                if (e.Data == null)
                    return;
                stderr.AppendLine(e.Data);
                onOutput?.Invoke(e.Data);
            };

            try
            {
                if (!process.Start())
                    return new ProcessResult { ExitCode = -1, Started = false };

                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                using var registration = cancellationToken.Register(() =>
                {
                    try
                    {
                        if (!process.HasExited)
                            process.Kill(entireProcessTree: true);
                    }
                    catch
                    {
                        // ignored
                    }
                });

                var completed = await Task.Run(() => process.WaitForExit((int)timeout.TotalMilliseconds), cancellationToken);
                if (!completed)
                {
                    try
                    {
                        process.Kill(entireProcessTree: true);
                    }
                    catch
                    {
                        // ignored
                    }

                    return new ProcessResult
                    {
                        ExitCode = -1,
                        StdOut = stdout.ToString(),
                        StdErr = stderr.ToString(),
                        TimedOut = true
                    };
                }

                return new ProcessResult
                {
                    ExitCode = process.ExitCode,
                    StdOut = stdout.ToString(),
                    StdErr = stderr.ToString()
                };
            }
            catch (Exception ex)
            {
                return new ProcessResult
                {
                    ExitCode = -1,
                    StdErr = ex.Message,
                    Started = false
                };
            }
        }

        public static Task<ProcessResult> RunCmdAsync(
            string command,
            string workingDirectory,
            TimeSpan timeout,
            Action<string>? onOutput = null,
            CancellationToken cancellationToken = default)
        {
            return RunAsync("cmd.exe", $"/c {command}", workingDirectory, timeout, onOutput, cancellationToken);
        }
    }
}
