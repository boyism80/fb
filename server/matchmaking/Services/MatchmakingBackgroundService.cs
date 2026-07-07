using Matchmaking.Core;
using Matchmaking.Model;
using Matchmaking.Options;
using Microsoft.Extensions.Options;

namespace Matchmaking.Services;

public sealed class MatchmakingBackgroundService : BackgroundService
{
    private readonly CharacterMatchMaker _matchMaker;
    private readonly MatchmakingOptions _options;
    private readonly ILogger<MatchmakingBackgroundService> _logger;

    public MatchmakingBackgroundService(
        CharacterMatchMaker matchMaker,
        IOptions<MatchmakingOptions> options,
        ILogger<MatchmakingBackgroundService> logger)
    {
        _matchMaker = matchMaker;
        _options = options.Value;
        _logger = logger;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var interval = TimeSpan.FromMilliseconds(Math.Max(1, _options.TickIntervalMs));

        while (!stoppingToken.IsCancellationRequested)
        {
            try
            {
                await _matchMaker.TickMatchmakingAsync(stoppingToken);
                _matchMaker.LogSaturatedQueues(_logger);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Matchmaking tick failed");
            }

            await Task.Delay(interval, stoppingToken);
        }
    }
}
