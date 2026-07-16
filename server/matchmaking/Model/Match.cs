namespace Matchmaking.Model;

public class Match<TEntry>
    where TEntry : IRegistryEntry
{
    public Match(IReadOnlyList<IReadOnlyList<Registry<TEntry>>> teams)
    {
        Teams = teams;
    }

    public Guid MatchId { get; set; }

    public uint MatchType { get; set; }

    public DateTime CreatedAt { get; set; }

    public IReadOnlyList<IReadOnlyList<Registry<TEntry>>> Teams { get; }

    public HashSet<string> ConfirmedEntryIds { get; } = new();

    public IReadOnlyList<Registry<TEntry>> AllRegistries => Teams.SelectMany(team => team).ToList();

    public IEnumerable<string> AllEntryIds => AllRegistries.SelectMany(registry => registry.Entries.Select(entry => entry.EntryId));
}
