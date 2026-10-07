using System.IO;
using Microsoft.Extensions.Configuration;

namespace MapEditor.Settings
{
    public enum ClientVersion
    {
        v550,
        v651,
    }

    public class PathSettings
    {
        public string RepositoryRoot { get; set; } = "";
        public string MapDirectory { get; set; } = "server/game/maps";
        public string TableDirectory { get; set; } = "resources/table";
    }

    public class ClientSettings
    {
        public string v550 { get; set; } = "";
        public string v651 { get; set; } = "";
    }

    public class McpSettings
    {
        public bool Enabled { get; set; } = true;
        public string Host { get; set; } = "127.0.0.1";
        public int Port { get; set; } = 5911;
        public string Path { get; set; } = "/mcp";
        public bool AllowWrite { get; set; } = false;
    }

    public class AppSettings
    {
        public PathSettings Paths { get; set; } = new PathSettings();
        public ClientSettings Client { get; set; } = new ClientSettings();
        public McpSettings Mcp { get; set; } = new McpSettings();

        public string Root { get; private set; } = "";
        public string MapDirectory => Path.Combine(Root, Paths.MapDirectory);
        public string TableDirectory => Path.Combine(Root, Paths.TableDirectory);

        /// <summary>
        /// appsettings.json, then appsettings.{MAPEDITOR_ENVIRONMENT}.json on top of it. Machine-specific values such
        /// as client directories belong in the environment file (e.g. appsettings.Local.json, not committed).
        /// </summary>
        public static AppSettings Load()
        {
            var builder = new ConfigurationBuilder()
                .SetBasePath(AppContext.BaseDirectory)
                .AddJsonFile("appsettings.json", optional: true);
            var environment = Environment.GetEnvironmentVariable("MAPEDITOR_ENVIRONMENT");
            if (string.IsNullOrWhiteSpace(environment) == false)
                builder.AddJsonFile($"appsettings.{environment}.json", optional: true);
            var configuration = builder.Build();

            var settings = new AppSettings();
            configuration.Bind(settings);

            if (string.IsNullOrWhiteSpace(settings.Paths.RepositoryRoot))
            {
                var dir = new DirectoryInfo(AppContext.BaseDirectory);
                while (dir != null && File.Exists(Path.Combine(dir.FullName, "fb.sln")) == false)
                    dir = dir.Parent;
                settings.Root = dir?.FullName ?? AppContext.BaseDirectory;
            }
            else
            {
                settings.Root = Path.GetFullPath(settings.Paths.RepositoryRoot);
            }
            return settings;
        }

        public string ClientDirectory(ClientVersion version)
        {
            return version == ClientVersion.v550 ? Client.v550 : Client.v651;
        }

        /// <summary>
        /// A version is editable only when its directory is configured and holds TILE.DAT.
        /// </summary>
        public bool IsEnabled(ClientVersion version)
        {
            var dir = ClientDirectory(version);
            if (string.IsNullOrWhiteSpace(dir))
                return false;

            return File.Exists(Path.Combine(dir, "TILE.DAT"));
        }
    }
}
