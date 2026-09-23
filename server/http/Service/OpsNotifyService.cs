namespace Http.Service
{
    public class OpsNotifyService
    {
        private readonly LogService _logService;
        private readonly DiscordNotifier _discord;

        public OpsNotifyService(LogService logService, DiscordNotifier discord)
        {
            _logService = logService;
            _discord = discord;
        }

        public async Task Notify(string action, string source, string actor, string target, uint? world, string detail)
        {
            action ??= string.Empty;
            source ??= string.Empty;
            actor ??= string.Empty;
            target ??= string.Empty;
            detail ??= string.Empty;
            await _logService.WriteAsync(string.IsNullOrEmpty(action) ? "ops" : action, new
            {
                action,
                source,
                actor,
                target,
                world,
                detail
            });
            await _discord.NotifyAsync(action, source, actor, target, world, detail);
        }
    }
}
