namespace Matchmaking.Model;

public sealed class ProposedMatchResult<TMember>
    where TMember : ITicketMember
{
    public Match<TMember> Match { get; init; }

    public DateTime ConfirmDeadline { get; init; }
}

public sealed class DissolvedMatchResult<TMember>
    where TMember : ITicketMember
{
    public Match<TMember> Match { get; init; }

    public DissolveReason Reason { get; init; }

    public IReadOnlyList<TicketOutcome<TMember>> Outcomes { get; init; }
}
