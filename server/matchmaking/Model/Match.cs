namespace Matchmaking.Model;

public class Match<TMember>
    where TMember : ITicketMember
{
    public Match(IReadOnlyList<IReadOnlyList<Ticket<TMember>>> teams)
    {
        Teams = teams;
    }

    public ulong Id { get; set; }

    public uint Type { get; set; }

    public bool Proposing { get; set; }

    public DateTime? ProposedAt { get; set; }

    public DissolvedMatchResult<TMember> DeferredDissolve { get; set; }

    public IReadOnlyList<IReadOnlyList<Ticket<TMember>>> Teams { get; }

    public HashSet<string> ConfirmedMemberIds { get; } = new();

    public IReadOnlyList<Ticket<TMember>> AllTickets => Teams.SelectMany(team => team).ToList();

    public IEnumerable<string> AllMemberIds => AllTickets.SelectMany(ticket => ticket.Members.Select(member => member.MemberId));
}
