namespace Matchmaking.Model;

public sealed class MatchmakingStatus
{
    public bool Queued { get; init; }

    public uint MatchType { get; init; }

    public ulong TicketId { get; init; }

    public ulong? PendingMatchId { get; init; }

    public DateTime? ConfirmDeadline { get; init; }
}
