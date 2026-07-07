using AutoMapper;
using Google.FlatBuffers;
using Http.Service;
using Matchmaking.Core;
using Matchmaking.Model;
using Matchmaking.Options;
using Microsoft.Extensions.Options;

namespace Matchmaking.Services;

public sealed class CharacterMatchMaker : MatchMaker<CharacterRegistryEntry>
{
    private const byte RegistryOutcomeExcluded = 0;
    private const byte RegistryOutcomeRequeued = 1;

    private readonly RabbitMqService _rabbitMqService;
    private readonly IMapper _mapper;
    private readonly ILogger<CharacterMatchMaker> _logger;

    public CharacterMatchMaker(
        IOptions<MatchmakingOptions> options,
        RabbitMqService rabbitMqService,
        IMapper mapper,
        ILogger<CharacterMatchMaker> logger)
        : base(options)
    {
        _rabbitMqService = rabbitMqService;
        _mapper = mapper;
        _logger = logger;

        MatchProposed += OnMatchProposedAsync;
        MatchReady += OnMatchReadyAsync;
        MatchDissolved += OnMatchDissolvedAsync;
    }

    private async Task OnMatchProposedAsync(
        ProposedMatchResult<CharacterRegistryEntry> args,
        CancellationToken cancellationToken)
    {
        var participants = args.Match.AllRegistries
            .SelectMany(registry => registry.Entries)
            .Select(entry => entry.EntryId)
            .ToList();

        _logger.LogInformation(
            "Match {MatchId} awaiting confirmation for matchType={MatchType} deadline={ConfirmDeadline} participants=[{Participants}] queue={QueueSnapshot}",
            args.Match.MatchId,
            args.Match.MatchType,
            args.ConfirmDeadline,
            string.Join(",", participants),
            DescribeQueues());

        var message = new fb.protocol.matchmaking.mq.Proposed
        {
            MatchId = args.Match.MatchId.ToString(),
            MatchType = args.Match.MatchType,
            ConfirmDeadline = args.ConfirmDeadline.ToString("yyyy-MM-dd HH:mm:ss"),
            Teams = BuildTeams(args.Match)
        };

        await PublishToWorldsAsync(args.Match, message, cancellationToken);
    }

    private async Task OnMatchReadyAsync(
        Match<CharacterRegistryEntry> match,
        CancellationToken cancellationToken)
    {
        _logger.LogInformation(
            "Match {MatchId} finalized for match type {MatchType}",
            match.MatchId,
            match.MatchType);

        var message = new fb.protocol.matchmaking.mq.Ready
        {
            MatchId = match.MatchId.ToString(),
            MatchType = match.MatchType,
            Teams = BuildTeams(match)
        };

        await PublishToWorldsAsync(match, message, cancellationToken);
    }

    private async Task OnMatchDissolvedAsync(
        DissolvedMatchResult<CharacterRegistryEntry> result,
        CancellationToken cancellationToken)
    {
        var outcomes = result.Outcomes.Select(outcome =>
            $"{outcome.RegistryId:N}:{(outcome.Requeued ? "requeued" : "excluded")}").ToList();

        _logger.LogInformation(
            "Match {MatchId} dissolved matchType={MatchType} reason={Reason} outcomes=[{Outcomes}] queue={QueueSnapshot}",
            result.Match.MatchId,
            result.Match.MatchType,
            result.Reason,
            string.Join(",", outcomes),
            DescribeQueues());

        var message = new fb.protocol.matchmaking.mq.Dissolved
        {
            MatchId = result.Match.MatchId.ToString(),
            MatchType = result.Match.MatchType,
            Reason = (byte)result.Reason,
            RegistryOutcomes = result.Outcomes.Select(outcome => new fb.protocol.matchmaking.RegistryOutcomeEntry
            {
                RegistryId = outcome.RegistryId.ToString(),
                Outcome = outcome.Requeued ? RegistryOutcomeRequeued : RegistryOutcomeExcluded,
                Entries = _mapper.Map<List<fb.protocol.matchmaking.RegistryEntry>>(outcome.Entries)
            }).ToList()
        };

        await PublishToWorldsAsync(result.Match, message, cancellationToken);
    }

    private async Task PublishToWorldsAsync(
        Match<CharacterRegistryEntry> match,
        IFlatBufferEx message,
        CancellationToken cancellationToken)
    {
        var worlds = match.AllRegistries
            .SelectMany(registry => registry.Entries)
            .Select(entry => entry.World)
            .Distinct();

        foreach (var world in worlds)
        {
            cancellationToken.ThrowIfCancellationRequested();

            try
            {
                var routingKey = $"fb.{world}.matchmaking";
                await _rabbitMqService.PublishAsync(
                    message,
                    "amq.direct",
                    routingKey,
                    cancellationToken);

                _logger.LogInformation(
                    "Published matchmaking message type={MessageType} exchange=amq.direct routingKey={RoutingKey} participants=[{Participants}]",
                    message.GetType().Name,
                    routingKey,
                    string.Join(",", match.AllRegistries.SelectMany(registry => registry.Entries).Select(entry => entry.EntryId)));
            }
            catch (Exception ex)
            {
                _logger.LogError(
                    ex,
                    "Failed to publish matchmaking message for world {World}",
                    world);
            }
        }
    }

    private List<fb.protocol.matchmaking.MatchTeam> BuildTeams(Match<CharacterRegistryEntry> match)
    {
        return match.Teams.Select(team => new fb.protocol.matchmaking.MatchTeam
        {
            Registries = _mapper.Map<List<fb.protocol.matchmaking.MatchRegistry>>(team.ToList())
        }).ToList();
    }
}
