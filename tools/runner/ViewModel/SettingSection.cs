namespace Runner.ViewModel
{
    public enum SettingSection
    {
        Directory,
        Infrastructure,
        Gateway,
        Login,
        Game,
        Internal,
        Marketplace,
        Matchmaking,
        AdminTool
    }

    public sealed class SettingNavItem
    {
        public SettingNavItem(string group, string title, SettingSection section)
        {
            Group = group;
            Title = title;
            Section = section;
        }

        public string Group { get; }
        public string Title { get; }
        public SettingSection Section { get; }
    }
}
