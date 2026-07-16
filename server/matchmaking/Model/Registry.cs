namespace Matchmaking.Model;

public sealed class Registry<TEntry> : IComparable<Registry<TEntry>>
    where TEntry : IRegistryEntry
{
    public Registry(Guid id, DateTime createdDateTime, IReadOnlyList<TEntry> entries)
    {
        Id = id;
        CreatedDateTime = createdDateTime;
        Entries = entries;
    }

    public Guid Id { get; }

    public DateTime CreatedDateTime { get; }

    public IReadOnlyList<TEntry> Entries { get; }

    public Skill Skill => SkillCalculator.ForRegistry(this);

    public int CompareTo(Registry<TEntry> other)
    {
        if (other == null)
        {
            return 1;
        }

        var timeComparison = CreatedDateTime.CompareTo(other.CreatedDateTime);
        if (timeComparison != 0)
        {
            return timeComparison;
        }

        return Id.CompareTo(other.Id);
    }
}
