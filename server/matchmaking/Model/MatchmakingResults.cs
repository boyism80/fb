namespace Matchmaking.Model;

public sealed class ProposedMatchResult<TEntry>
    where TEntry : IRegistryEntry
{
    public Match<TEntry> Match { get; init; }

    public DateTime ConfirmDeadline { get; init; }
}

public sealed class DissolvedMatchResult<TEntry>
    where TEntry : IRegistryEntry
{
    public Match<TEntry> Match { get; init; }

    public DissolveReason Reason { get; init; }

    public IReadOnlyList<RegistryOutcomeNotification<TEntry>> Outcomes { get; init; }
}
