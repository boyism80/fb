namespace Http.Service
{
    public class ShutdownService
    {
        private readonly LogService _logService;
        private readonly DiscordNotifier _discord;
        private readonly RabbitMqService _rabbitMqService;

        public ShutdownService(LogService logService, DiscordNotifier discord, RabbitMqService rabbitMqService)
        {
            _logService = logService;
            _discord = discord;
            _rabbitMqService = rabbitMqService;
        }

        public async Task Shutdown(string actor, string source)
        {
            actor ??= string.Empty;
            source ??= string.Empty;
            await _logService.WriteAsync("shutdown", new
            {
                action = "shutdown",
                source,
                actor
            });
            await _discord.NotifyAsync("shutdown", source, actor, null, null, null);
            await _rabbitMqService.PublishAsync(new fb.protocol._internal.response.Shutdown(), "amq.direct", "fb.global");
        }
    }
}
