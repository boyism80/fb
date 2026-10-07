using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using MapEditor.Settings;
using MapEditor.ViewModel;

namespace MapEditor.Mcp
{
    /// <summary>
    /// Streamable HTTP MCP server inside the editor process, configured by appsettings.json "Mcp".
    /// </summary>
    public static class McpHost
    {
        public static async Task<WebApplication> Start(McpSettings settings, MainWindowViewModel editor)
        {
            if (settings.Enabled == false)
            {
                editor.McpStatus = "꺼짐 (appsettings Mcp.Enabled=false)";
                return null;
            }

            var url = $"http://{settings.Host}:{settings.Port}";
            var path = settings.Path.StartsWith('/') ? settings.Path : "/" + settings.Path;
            try
            {
                var builder = WebApplication.CreateSlimBuilder();
                builder.WebHost.UseUrls(url);
                builder.Logging.ClearProviders();
                builder.Services.AddSingleton(editor);
                builder.Services.AddMcpServer()
                                .WithHttpTransport()
                                .WithTools<MapTools>();

                var app = builder.Build();
                app.MapMcp(path);
                await app.StartAsync();

                var loopback = settings.Host == "127.0.0.1" || settings.Host == "localhost" || settings.Host == "::1";
                editor.McpStatus = loopback ? $"{url}{path}" : $"{url}{path} (경고: 루프백 아님)";
                return app;
            }
            catch (Exception e)
            {
                editor.McpStatus = $"시작 실패: {e.Message}";
                return null;
            }
        }
    }
}
