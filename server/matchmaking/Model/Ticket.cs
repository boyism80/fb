namespace Matchmaking.Model;

public sealed class Ticket<TMember> : IComparable<Ticket<TMember>>
    where TMember : ITicketMember
{
    public Ticket(ulong id, uint matchType, DateTime createdAt, IReadOnlyList<TMember> members)
    {
        Id = id;
        MatchType = matchType;
        CreatedAt = createdAt;
        Members = members;
    }

    public ulong Id { get; }

    public uint MatchType { get; }

    public DateTime CreatedAt { get; }

    public IReadOnlyList<TMember> Members { get; }

    public Skill Skill => SkillCalculator.ForTicket(this);

    public int CompareTo(Ticket<TMember> other)
    {
        if (other == null)
        {
            return 1;
        }

        var timeComparison = CreatedAt.CompareTo(other.CreatedAt);
        if (timeComparison != 0)
        {
            return timeComparison;
        }

        return Id.CompareTo(other.Id);
    }
}
