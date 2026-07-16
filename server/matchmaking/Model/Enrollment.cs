namespace Matchmaking.Model;

public enum EnrollmentState
{
    Waiting,
    Pending
}

public sealed class EnrollmentRef
{
    public EnrollmentRef(EnrollmentState state, uint matchType, Guid registryId, Guid? matchId = null)
    {
        State = state;
        MatchType = matchType;
        RegistryId = registryId;
        MatchId = matchId;
    }

    public EnrollmentState State { get; }

    public uint MatchType { get; }

    public Guid RegistryId { get; }

    public Guid? MatchId { get; }
}

public enum DissolveReason
{
    Timeout,
    Decline
}

public sealed class RegistryOutcomeNotification<TEntry>
    where TEntry : IRegistryEntry
{
    public Guid RegistryId { get; init; }

    public bool Requeued { get; init; }

    public IReadOnlyList<TEntry> Entries { get; init; }
}
