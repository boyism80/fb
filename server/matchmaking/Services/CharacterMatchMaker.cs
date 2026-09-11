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
        : base(options, logger)
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
        _logger.LogInformation(
            "Match {MatchId} awaiting confirmation for match type {MatchType}, deadline {ConfirmDeadline}, entries {Entries}",
            args.Match.MatchId,
            args.Match.MatchType,
            args.ConfirmDeadline,
            string.Join(",", args.Match.AllEntryIds));

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
            "Match {MatchId} finalized for match type {MatchType}, entries {Entries}",
            match.MatchId,
            match.MatchType,
            string.Join(",", match.AllEntryIds));

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
        _logger.LogInformation(
            "Match {MatchId} dissolved due to {Reason}, outcomes {Outcomes}",
            result.Match.MatchId,
            result.Reason,
            string.Join(
                " | ",
                result.Outcomes.Select(outcome => string.Format(
                    "{0}={1} [{2}]",
                    outcome.RegistryId,
                    outcome.Requeued ? "requeued" : "dropped",
                    string.Join(",", outcome.Entries.Select(entry => entry.EntryId))))));

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
                await _rabbitMqService.PublishFanoutAsync(
                    message,
                    "matchmaking",
                    world,
                    cancellationToken);
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
