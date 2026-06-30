namespace AdminTool.Services
{
    using Microsoft.JSInterop;

    public class ThemeService
    {
        public const string Dark = "dark";
        public const string Light = "light";
        public const string System = "system";

        public string Current { get; private set; } = Dark;

        public event Action Changed;

        public async Task InitializeAsync(IJSRuntime js)
        {
            Current = await js.InvokeAsync<string>("fbAdminTheme.get");
            Changed?.Invoke();
        }

        public async Task SetThemeAsync(IJSRuntime js, string theme)
        {
            if (theme != Light && theme != System)
                theme = Dark;

            Current = theme;
            await js.InvokeVoidAsync("fbAdminTheme.set", theme);
            Changed?.Invoke();
        }
    }
}
