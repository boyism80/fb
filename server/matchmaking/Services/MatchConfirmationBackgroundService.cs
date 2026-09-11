using Matchmaking.Core;
using Matchmaking.Model;

namespace Matchmaking.Services;

public sealed class MatchConfirmationBackgroundService : BackgroundService
{
    private const int HeartbeatSeconds = 30;

    private readonly MatchMaker<CharacterRegistryEntry> _matchMaker;
    private readonly ILogger<MatchConfirmationBackgroundService> _logger;

    public MatchConfirmationBackgroundService(
        MatchMaker<CharacterRegistryEntry> matchMaker,
        ILogger<MatchConfirmationBackgroundService> logger)
    {
        _matchMaker = matchMaker;
        _logger = logger;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var elapsedSeconds = 0;

        while (!stoppingToken.IsCancellationRequested)
        {
            try
            {
                await _matchMaker.TickConfirmationAsync(stoppingToken);

                elapsedSeconds++;
                if (elapsedSeconds % HeartbeatSeconds == 0)
                {
                    _matchMaker.LogQueueState("heartbeat");
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Match confirmation tick failed");
            }

            await Task.Delay(TimeSpan.FromSeconds(1), stoppingToken);
        }
    }
}
