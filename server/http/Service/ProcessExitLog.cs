using System.Runtime.InteropServices;
using System.Text;

namespace Http.Service
{
    public static class ProcessExitLog
    {
        private static readonly object Sync = new();
        private static readonly UTF8Encoding Encoding = new(false);
        private static readonly List<PosixSignalRegistration> Registrations = new();
        private static string _service = "unknown";
        private static bool _installed;

        public static void Install(string service)
        {
            lock (Sync)
            {
                if (_installed)
                {
                    return;
                }

                _installed = true;
                _service = service;
            }

            AppDomain.CurrentDomain.UnhandledException += (_, e) =>
                Write("FATAL", $"unhandled exception (terminating={e.IsTerminating}): {e.ExceptionObject}");
            AppDomain.CurrentDomain.ProcessExit += (_, _) =>
                Write("WARN", $"process exit code={Environment.ExitCode}");

            foreach (var signal in new[] { PosixSignal.SIGINT, PosixSignal.SIGQUIT, PosixSignal.SIGTERM })
            {
                Registrations.Add(PosixSignalRegistration.Create(signal, context =>
                    Write("WARN", $"signal {Describe(context.Signal)}")));
            }

            Write("INFO", $"process started pid={Environment.ProcessId}");
        }

        public static void Write(string level, string message)
        {
            var now = DateTime.UtcNow;
            var line = $"{$"[{level}]",-7} {now:yyyy-MM-dd HH:mm:ss.ffffff} [{_service}] {message}";
            lock (Sync)
            {
                try
                {
                    Directory.CreateDirectory("logs");
                    File.AppendAllText(Path.Combine("logs", $"{now:yyyy-MM-dd}-{_service}.log"), line + Environment.NewLine, Encoding);
                }
                catch (IOException)
                {
                }
                catch (UnauthorizedAccessException)
                {
                }
            }

            Console.Error.WriteLine(line);
        }

        private static string Describe(PosixSignal signal) => signal switch
        {
            PosixSignal.SIGINT => "SIGINT (Ctrl+C)",
            PosixSignal.SIGQUIT => "SIGQUIT (Ctrl+Break)",
            PosixSignal.SIGTERM when OperatingSystem.IsWindows() => "SIGTERM (console window closed, logoff or shutdown)",
            _ => signal.ToString()
        };
    }
}
