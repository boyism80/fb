using System.Text;
using System.Windows;
using Microsoft.AspNetCore.Builder;
using MapEditor.Mcp;
using MapEditor.Settings;
using MapEditor.ViewModel;

namespace MapEditor
{
    public partial class App : Application
    {
        private WebApplication _mcp;

        protected override async void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);

            var settings = AppSettings.Load();
            var user = UserSettings.Load(out var userError);
            var editor = new MainWindowViewModel(settings, user);
            var window = new MainWindow { DataContext = editor };
            MainWindow = window;
            window.Show();

            await editor.Initialize();
            if (userError != null)
                editor.StatusText = userError;
            _mcp = await McpHost.Start(settings.Mcp, editor);
        }

        protected override void OnExit(ExitEventArgs e)
        {
            _mcp?.StopAsync().Wait(TimeSpan.FromSeconds(2));
            base.OnExit(e);
        }
    }
}
