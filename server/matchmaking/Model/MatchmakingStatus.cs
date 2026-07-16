namespace Matchmaking.Model;

public sealed class MatchmakingStatus
{
    public bool InQueue { get; init; }

    public uint MatchType { get; init; }

    public Guid RegistryId { get; init; }

    public Guid? PendingMatchId { get; init; }

    public DateTime? ConfirmDeadline { get; init; }
}
