using System.Net.Http.Headers;
using System.Text;
using Newtonsoft.Json;

namespace Http.Service
{
    public class DiscordNotifier
    {
        private const int ContentLimit = 1800;
        private readonly HttpClient _http = new() { Timeout = TimeSpan.FromSeconds(10) };
        private readonly ILogger<DiscordNotifier> _logger;
        private readonly string _token;
        private readonly string _channelId;
        private readonly bool _enabled;

        public DiscordNotifier(IConfiguration configuration, ILogger<DiscordNotifier> logger)
        {
            _logger = logger;
            _token = Environment.GetEnvironmentVariable("DISCORD_BOT_TOKEN")?.Trim() ?? string.Empty;
            if (string.IsNullOrEmpty(_token))
                _token = configuration["Discord:BotToken"]?.Trim() ?? string.Empty;
            if (_token.StartsWith("Bot ", StringComparison.Ordinal))
                _token = _token.Substring(4).Trim();
            _channelId = Environment.GetEnvironmentVariable("DISCORD_CHANNEL_ID")?.Trim() ?? string.Empty;
            if (string.IsNullOrEmpty(_channelId))
                _channelId = configuration["Discord:ChannelId"]?.Trim() ?? string.Empty;
            _enabled = string.IsNullOrWhiteSpace(_token) == false && string.IsNullOrWhiteSpace(_channelId) == false;
            if (_enabled == false)
                _logger.LogInformation("Discord ops notify disabled (token or channel is empty)");
        }

        public async Task NotifyAsync(string action, string source, string actor, string target, uint? world, string detail)
        {
            if (_enabled == false)
                return;

            try
            {
                var body = new StringBuilder();
                body.Append('[').Append(action).Append(']');
                if (world.HasValue)
                    body.Append(" world=").Append(world.Value);
                body.Append(" source=").Append(source ?? string.Empty);
                body.Append('\n').Append("actor: ").Append(actor ?? string.Empty);
                if (string.IsNullOrEmpty(target) == false)
                    body.Append('\n').Append("target: ").Append(target);
                if (string.IsNullOrEmpty(detail) == false)
                    body.Append('\n').Append(detail);

                var content = body.ToString();
                if (content.Length > ContentLimit)
                    content = content.Substring(0, ContentLimit);

                var payload = JsonConvert.SerializeObject(new { content });
                using var request = new HttpRequestMessage(
                    HttpMethod.Post,
                    $"https://discord.com/api/v10/channels/{_channelId}/messages");
                request.Headers.Authorization = new AuthenticationHeaderValue("Bot", _token);
                request.Headers.TryAddWithoutValidation("User-Agent", "fb-ops-notify/1.0");
                request.Content = new StringContent(payload, Encoding.UTF8, "application/json");

                using var response = await _http.SendAsync(request);
                if (response.IsSuccessStatusCode == false)
                {
                    var error = await response.Content.ReadAsStringAsync();
                    if (error.Length > 500)
                        error = error.Substring(0, 500);
                    _logger.LogWarning("Discord ops notify failed: {Status} {Body}", (int)response.StatusCode, error);
                }
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Discord ops notify failed for action {Action}", action);
            }
        }
    }
}
