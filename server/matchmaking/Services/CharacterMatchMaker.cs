using AutoMapper;
using Google.FlatBuffers;
using Http.Service;
using Matchmaking.Core;
using Matchmaking.Model;
using Matchmaking.Options;
using Microsoft.Extensions.Options;

namespace Matchmaking.Services;

public sealed class CharacterMatchMaker : MatchMaker<CharacterTicketMember>
{
    private const byte TicketOutcomeExcluded = 0;
    private const byte TicketOutcomeRequeued = 1;

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
        TicketRemoved += OnTicketRemovedAsync;
    }

    private async Task OnMatchProposedAsync(
        ProposedMatchResult<CharacterTicketMember> args,
        CancellationToken cancellationToken)
    {
        _logger.LogInformation(
            "Match {MatchId} awaiting confirmation for match type {MatchType}, deadline {ConfirmDeadline}, members {Members}",
            args.Match.Id,
            args.Match.Type,
            args.ConfirmDeadline,
            string.Join(",", args.Match.AllMemberIds));

        var message = new fb.protocol.matchmaking.mq.MatchProposed
        {
            MatchId = args.Match.Id,
            MatchType = args.Match.Type,
            ConfirmDeadline = args.ConfirmDeadline.ToString("yyyy-MM-dd HH:mm:ss"),
            Teams = BuildTeams(args.Match)
        };

        await PublishToWorldsAsync(args.Match.AllTickets.SelectMany(ticket => ticket.Members), message, cancellationToken);
    }

    private async Task OnMatchReadyAsync(
        Match<CharacterTicketMember> match,
        CancellationToken cancellationToken)
    {
        _logger.LogInformation(
            "Match {MatchId} ready for match type {MatchType}, members {Members}",
            match.Id,
            match.Type,
            string.Join(",", match.AllMemberIds));

        var message = new fb.protocol.matchmaking.mq.MatchReady
        {
            MatchId = match.Id,
            MatchType = match.Type,
            Teams = BuildTeams(match)
        };

        await PublishToWorldsAsync(match.AllTickets.SelectMany(ticket => ticket.Members), message, cancellationToken);
    }

    private async Task OnMatchDissolvedAsync(
        DissolvedMatchResult<CharacterTicketMember> result,
        CancellationToken cancellationToken)
    {
        _logger.LogInformation(
            "Match {MatchId} dissolved due to {Reason}, outcomes {Outcomes}",
            result.Match.Id,
            result.Reason,
            string.Join(
                " | ",
                result.Outcomes.Select(outcome => string.Format(
                    "{0}={1} [{2}]",
                    outcome.TicketId,
                    outcome.Requeued ? "requeued" : "dropped",
                    string.Join(",", outcome.Members.Select(member => member.MemberId))))));

        var message = new fb.protocol.matchmaking.mq.MatchDissolved
        {
            MatchId = result.Match.Id,
            MatchType = result.Match.Type,
            Reason = (byte)result.Reason,
            TicketOutcomes = result.Outcomes.Select(outcome => new fb.protocol.matchmaking.TicketOutcome
            {
                TicketId = outcome.TicketId,
                Outcome = outcome.Requeued ? TicketOutcomeRequeued : TicketOutcomeExcluded,
                Members = _mapper.Map<List<fb.protocol.matchmaking.TicketMember>>(outcome.Members)
            }).ToList()
        };

        await PublishToWorldsAsync(result.Match.AllTickets.SelectMany(ticket => ticket.Members), message, cancellationToken);
    }

    private async Task OnTicketRemovedAsync(
        Ticket<CharacterTicketMember> ticket,
        CancellationToken cancellationToken)
    {
        _logger.LogInformation(
            "Ticket {TicketId} of match type {MatchType} removed, notifying members {Members}",
            ticket.Id,
            ticket.MatchType,
            string.Join(",", ticket.Members.Select(member => member.MemberId)));

        var message = new fb.protocol.matchmaking.mq.TicketRemoved
        {
            MatchType = ticket.MatchType,
            TicketId = ticket.Id,
            Members = _mapper.Map<List<fb.protocol.matchmaking.TicketMember>>(ticket.Members)
        };

        await PublishToWorldsAsync(ticket.Members, message, cancellationToken);
    }

    private async Task PublishToWorldsAsync(
        IEnumerable<CharacterTicketMember> members,
        IFlatBufferEx message,
        CancellationToken cancellationToken)
    {
        var worlds = members
            .Select(member => member.World)
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

    private List<fb.protocol.matchmaking.MatchTeam> BuildTeams(Match<CharacterTicketMember> match)
    {
        return match.Teams.Select(team => new fb.protocol.matchmaking.MatchTeam
        {
            Tickets = _mapper.Map<List<fb.protocol.matchmaking.Ticket>>(team.ToList())
        }).ToList();
    }
}
