using Http.Service;
using Matchmaking.Core;
using Matchmaking.Model;
using Matchmaking.Options;
using Microsoft.Extensions.Options;

namespace Matchmaking.Services;

public sealed class MatchmakingBackgroundService : BackgroundService
{
    private readonly MatchMaker<CharacterTicketMember> _matchMaker;
    private readonly ServerStateService _serverStateService;
    private readonly MatchmakingOptions _options;
    private readonly ILogger<MatchmakingBackgroundService> _logger;

    public MatchmakingBackgroundService(
        MatchMaker<CharacterTicketMember> matchMaker,
        ServerStateService serverStateService,
        IOptions<MatchmakingOptions> options,
        ILogger<MatchmakingBackgroundService> logger)
    {
        _matchMaker = matchMaker;
        _serverStateService = serverStateService;
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
                var hostIds = (await _serverStateService.ListLiveCrossServers())
                    .Select(server => server.Id)
                    .ToList();
                await _matchMaker.TickMatchmakingAsync(hostIds, stoppingToken);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Matchmaking tick failed");
            }

            await Task.Delay(interval, stoppingToken);
        }
    }
}
