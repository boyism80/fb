namespace Matchmaking.Model;

public enum TicketState
{
    Queued,
    Proposed
}

public sealed class TicketRef
{
    public TicketRef(TicketState state, uint matchType, ulong ticketId, ulong? matchId = null)
    {
        State = state;
        MatchType = matchType;
        TicketId = ticketId;
        MatchId = matchId;
    }

    public TicketState State { get; }

    public uint MatchType { get; }

    public ulong TicketId { get; }

    public ulong? MatchId { get; }
}

public enum DissolveReason
{
    Timeout,
    Decline
}

public sealed class TicketOutcome<TMember>
    where TMember : ITicketMember
{
    public ulong TicketId { get; init; }

    public bool Requeued { get; init; }

    public IReadOnlyList<TMember> Members { get; init; }
}
