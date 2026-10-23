using System.Net.Http.Headers;
using System.Text;
using System.Threading.Channels;
using Newtonsoft.Json;

namespace Http.Service
{
    public class DiscordNotifier : IHostedService, IAsyncDisposable
    {
        private const int ContentLimit = 1800;
        private const int QueueCapacity = 1000;
        private readonly HttpClient _http = new() { Timeout = TimeSpan.FromSeconds(10) };
        private readonly ILogger<DiscordNotifier> _logger;
        private readonly string _token;
        private readonly string _channelId;
        private readonly bool _enabled;
        private readonly Channel<Notice> _pending = Channel.CreateBounded<Notice>(new BoundedChannelOptions(QueueCapacity)
        {
            FullMode = BoundedChannelFullMode.DropOldest,
            SingleReader = true,
            SingleWriter = false
        });
        private readonly CancellationTokenSource _stopping = new();
        private Task _run = Task.CompletedTask;
        private long _dropped;

        public DiscordNotifier(IConfiguration configuration, ILogger<DiscordNotifier> logger)
        {
            _logger = logger;
            _token = Environment.GetEnvironmentVariable("DISCORD_BOT_TOKEN")?.Trim() ?? string.Empty;
            if (string.IsNullOrEmpty(_token))
            {
                _token = configuration["Discord:BotToken"]?.Trim() ?? string.Empty;
            }

            if (_token.StartsWith("Bot ", StringComparison.Ordinal))
            {
                _token = _token.Substring(4).Trim();
            }

            _channelId = Environment.GetEnvironmentVariable("DISCORD_CHANNEL_ID")?.Trim() ?? string.Empty;
            if (string.IsNullOrEmpty(_channelId))
            {
                _channelId = configuration["Discord:ChannelId"]?.Trim() ?? string.Empty;
            }

            _enabled = string.IsNullOrWhiteSpace(_token) == false && string.IsNullOrWhiteSpace(_channelId) == false;
            if (_enabled == false)
            {
                _logger.LogInformation("Discord ops notify disabled (token or channel is empty)");
            }
        }

        public Task StartAsync(CancellationToken cancellationToken)
        {
            if (_enabled)
            {
                _run = Task.Run(() => SendLoop(_stopping.Token));
            }

            return Task.CompletedTask;
        }

        public async Task StopAsync(CancellationToken cancellationToken)
        {
            _stopping.Cancel();
            _pending.Writer.TryComplete();
            try
            {
                await _run.WaitAsync(TimeSpan.FromSeconds(2), cancellationToken);
            }
            catch (Exception)
            {
            }
        }

        public Task NotifyAsync(string action, string source, string actor, string target, uint? world, string detail)
        {
            if (_enabled == false)
            {
                return Task.CompletedTask;
            }

            if (_pending.Reader.Count >= QueueCapacity)
            {
                var dropped = Interlocked.Increment(ref _dropped);
                if (dropped == 1 || dropped % 100 == 0)
                {
                    _logger.LogWarning("Dropped {Dropped} Discord notices because the queue is full", dropped);
                }
            }

            _pending.Writer.TryWrite(new Notice(action, source, actor, target, world, detail));
            return Task.CompletedTask;
        }

        private async Task SendLoop(CancellationToken stoppingToken)
        {
            try
            {
                await foreach (var notice in _pending.Reader.ReadAllAsync(stoppingToken))
                {
                    await SendAsync(notice);
                }
            }
            catch (OperationCanceledException)
            {
            }
        }

        private async Task SendAsync(Notice notice)
        {
            try
            {
                var body = new StringBuilder();
                body.Append('[').Append(notice.Action).Append(']');
                if (notice.World.HasValue)
                {
                    body.Append(" world=").Append(notice.World.Value);
                }

                body.Append(" source=").Append(notice.Source ?? string.Empty);
                body.Append('\n').Append("actor: ").Append(notice.Actor ?? string.Empty);
                if (string.IsNullOrEmpty(notice.Target) == false)
                {
                    body.Append('\n').Append("target: ").Append(notice.Target);
                }

                if (string.IsNullOrEmpty(notice.Detail) == false)
                {
                    body.Append('\n').Append(notice.Detail);
                }

                var content = body.ToString();
                if (content.Length > ContentLimit)
                {
                    content = content.Substring(0, ContentLimit);
                }

                var payload = JsonConvert.SerializeObject(new { content });
                using var request = new HttpRequestMessage(
                    HttpMethod.Post,
                    $"https://discord.com/api/v10/channels/{_channelId}/messages");
                request.Headers.Authorization = new AuthenticationHeaderValue("Bot", _token);
                request.Headers.TryAddWithoutValidation("User-Agent", "fb-ops-notify/1.0");
                request.Content = new StringContent(payload, Encoding.UTF8, "application/json");

                using var response = await _http.SendAsync(request, _stopping.Token);
                if (response.IsSuccessStatusCode == false)
                {
                    var error = await response.Content.ReadAsStringAsync(_stopping.Token);
                    if (error.Length > 500)
                    {
                        error = error.Substring(0, 500);
                    }

                    _logger.LogWarning("Discord ops notify failed: {Status} {Body}", (int)response.StatusCode, error);
                }
            }
            catch (OperationCanceledException) when (_stopping.IsCancellationRequested)
            {
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Discord ops notify failed for action {Action}", notice.Action);
            }
        }

        public async ValueTask DisposeAsync()
        {
            await StopAsync(CancellationToken.None);
            _stopping.Dispose();
            _http.Dispose();
        }

        private readonly record struct Notice(string Action, string Source, string Actor, string Target, uint? World, string Detail);
    }
}
